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

// used for syncing updating the world and drawing it
extern std::mutex world_mutex;
// used for syncing the above mutex
extern std::condition_variable world_cv;
extern std::atomic<bool> world_updated;

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
