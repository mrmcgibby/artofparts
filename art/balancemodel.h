#ifndef BALANCEMODEL_H
#define BALANCEMODEL_H

#include <vector>
#include <string>
#include <deque>

#include <QAbstractListModel>
#include <QScopedPointer>
#include <QSize>
#include <QPoint>
#include <QTimer>

#include "optimizer.h"
#include "box.h"

class BalanceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)

public:
    BalanceModel();
    virtual ~BalanceModel();
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void addBox(QString name, double size);
    Q_INVOKABLE void addDesired(QPoint p, int index = -1);
    Q_INVOKABLE void optimize();

    QSize size() { return m_size; }
    void setSize(const QSize& size);

    int interval() { return m_timer.interval(); }
    void setInterval(int interval);

    QHash<int, QByteArray> roleNames() const;

signals:
    void sizeChanged(QSize);
    void intervalChanged(int);

private slots:
    void handle_new_best(double score);
    void handle_next();

private:
    QSize m_size;
    QTimer m_timer;

    QScopedPointer<Optimizer> m_optimizer;
    std::vector<box> m_boxes, m_best;
    std::deque<std::vector<box>> m_upcoming;
};

#endif // BALANCEMODEL_H
