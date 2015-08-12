#include <QMutexLocker>

#include "optimizer.h"

using namespace std;

Optimizer::Optimizer(vector<box> boxes, box::point origin, box::point extent)
    : m_boxes(boxes)
    , m_origin(origin)
    , m_extent(extent)
{
}

Optimizer::~Optimizer()
{

}

void Optimizer::run()
{
    double	best_score;
    bool	best = false;

    while (true) {
        if (isInterruptionRequested()) {
            return;
        }
        random_shuffle(m_boxes.begin(), m_boxes.end());

        box test = m_boxes[0];
        test.map(m_extent, m_origin);
        for (unsigned i = 1; i < m_boxes.size(); ++i) {
            test.append(m_boxes[i]);
        }
        double score = test.score();
        if (!best || score < best_score) {
            best_score = score;
            best = true;

            {
                QMutexLocker lock(&m_best_lock);
                m_best = test.flat();
            }

            emit new_best(best_score);
        }
    }
}

std::vector<box> Optimizer::best()
{
    QMutexLocker lock(&m_best_lock);
    return m_best;
}
