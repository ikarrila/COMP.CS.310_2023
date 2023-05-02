#ifndef GRTIMER_HH 
#define GRTIMER_HH

#include "tuni.hh"
TUNI_WARN_OFF()
#include <condition_variable>
#include <mutex>
#include <QObject>
#include <QGraphicsPixmapItem>
TUNI_WARN_ON()

// helper class with two purposes:
// 1. generate periodic Qt Timer tick to redraw graphics
// 2. get notification of the application closing
//

std::mutex world_mutex;
std::condition_variable world_cv;
bool world_updated = false;

class GrTimer : public QObject
{
    Q_OBJECT;
public:
    explicit GrTimer( QGraphicsPixmapItem* );

public slots:
    void closing();

protected:
    void timerEvent(QTimerEvent*) override;

private:
    QGraphicsPixmapItem* target_;
    int timerID_;
};

#endif 
