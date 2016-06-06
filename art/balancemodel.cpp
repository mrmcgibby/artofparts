#include <QRectF>
#include <QPoint>
#include <QThread>

#include "balancemodel.h"

using namespace std;

BalanceModel::BalanceModel()
    : m_optimizer(NULL)
{
    m_timer.start(1000);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(handle_next()));
}

BalanceModel::~BalanceModel()
{
    if (m_optimizer) {
        m_optimizer->requestInterruption();
        m_optimizer->wait();
    }
}

int BalanceModel::rowCount(const QModelIndex&) const
{
    return m_best.size();
}

QHash<int, QByteArray> BalanceModel::roleNames() const
{
    QHash<int, QByteArray> rval;
    rval[Qt::DisplayRole] = "display";
    rval[Qt::UserRole+0] = "name";
    rval[Qt::UserRole+1] = "desired";
    return rval;
}

QVariant BalanceModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int) m_best.size()) {
        return QVariant();
    }

    switch (role) {
	case Qt::DisplayRole: {
		QRectF rval;
		box b = m_best[index.row()];
		rval.setX(b.origin(0));
		rval.setY(b.origin(1));
		rval.setWidth(b.extent(0));
		rval.setHeight(b.extent(1));
		return rval;
	}
	case Qt::UserRole+0: {
		return QString(m_best[index.row()].name().c_str());
	}
	case Qt::UserRole+1: {
		box b = m_best[index.row()];
		if (b.contains_desired()) {
			box::point d = b.desired();
			return QPointF(d[0], d[1]);
		} else {
			return QPointF(0, 0);
		}
	}
	default: {
		return QVariant();
	}
    }
}

void BalanceModel::clear()
{
    beginResetModel();
    m_boxes.clear();
    endResetModel();
}

void BalanceModel::addBox(QString name, double size)
{
    auto i = find_if(m_boxes.begin(), m_boxes.end(), [name](const box& b) {
        return b.name() == name.toStdString();
    });

    if (i == m_boxes.end()) {
        m_boxes.push_back(box(size, name.toStdString()));
    } else {
        *i = box(size + i->size(), name.toStdString());
    }
}

void BalanceModel::addDesired(QPoint p, int index)
{
    if (p == QPoint(0,0)) {
        return;
    }

    if (index == -1) {
        index = m_boxes.size()-1;
    }

    box::point b;
    b[0] = p.x();
    b[1] = p.y();
    m_boxes[index].add_desired(b);
}

void BalanceModel::optimize()
{
    if (m_optimizer) {
        m_optimizer->requestInterruption();
        m_optimizer->wait();
    }
    box::point origin;
    origin[0] = 0;
    origin[1] = 0;
    box::point extent;
    extent[0] = m_size.width();
    extent[1] = m_size.height();
    m_optimizer.reset(new Optimizer(m_boxes, origin, extent));
    connect(m_optimizer.data(), SIGNAL(new_best(double)),
            this, SLOT(handle_new_best(double)));
    m_optimizer->start();
}

void BalanceModel::handle_new_best(double)
{
    vector<box> new_best = m_optimizer->best();
    std::sort(new_best.begin(), new_best.end(), [](const box& a, const box& b) {
       return a.name() < b.name();
    });

    if (new_best.size() != m_best.size()) {
        beginResetModel();
        m_best = new_best;
        m_upcoming.clear();
        endResetModel();
    } else {
        m_upcoming.push_back(new_best);
    }
}

void BalanceModel::handle_next()
{
    if (m_upcoming.size() == 0) {
        return;
    }

    m_best = m_upcoming.front();
    m_upcoming.pop_front();

    emit dataChanged(createIndex(0,0), createIndex(m_best.size()-1,0));
}

void BalanceModel::setSize(const QSize& size)
{
    m_size = size;
    emit sizeChanged(size);
}

void BalanceModel::setInterval(int interval)
{
    m_timer.start(interval);
    emit intervalChanged(m_timer.interval());
}
