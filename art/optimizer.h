#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>

#include <QMutex>
#include <QThread>

#include "box.h"

class Optimizer : public QThread
{
    Q_OBJECT
public:
    Optimizer(std::vector<box> boxes, box::point origin, box::point extent);
    virtual ~Optimizer();

    virtual void run();

    std::vector<box> best();

signals:
    void new_best(double score);

private:
    std::vector<box> m_boxes, m_best;
    QMutex m_best_lock;
    box::point m_origin, m_extent;
};

#endif // OPTIMIZER_H
