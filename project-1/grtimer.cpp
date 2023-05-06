#include "world.hh"
#include "graphics.hh"
#include "grtimer.hh"

#include "tuni.hh"
TUNI_WARN_OFF()
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
TUNI_WARN_ON()

std::mutex world_mutex;
std::condition_variable world_cv;
std::atomic<bool> world_updated = false;

GrTimer::GrTimer( QGraphicsPixmapItem* target )
    : QObject(), target_(target)
{
    qDebug() << "GrTimer started";
    timerID_ = startTimer(config::graphics_tick);
}

void GrTimer::closing()
{
    qDebug() << "GrTimer closing";
    world::running = false;
    killTimer( timerID_ );
}

void GrTimer::timerEvent(QTimerEvent*)
{
    qDebug() << "GrTimer tick";
    //Added synchronisation with update thread
    std::unique_lock<std::mutex> lock(world_mutex);
    world_cv.wait(lock, [&]() { return world_updated.load(); });

    graphics::draw_board();
    target_->setPixmap( *(graphics::current_pixmap) );
    target_->scene()->update();

    world_updated = false;

}
