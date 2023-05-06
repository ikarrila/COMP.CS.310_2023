//
// Conway's Game of Life
// https://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
// Course template, COMP.CS.320 Concurrency, spring 2023
// license: Free to use for any purpose in tuni.fi teaching,
// other uses: https://creativecommons.org/licenses/by-nc-sa/4.0/
//
#include "config.hh"
#include "world.hh"
#include "graphics.hh"
#include "grtimer.hh"
#include "thread_pool.hh"

#include "tuni.hh"
TUNI_WARN_OFF()
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
TUNI_WARN_ON()

#include <thread>
#include <chrono>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // data model of the world
    world::init();

    // Qt pixmap of the world
    graphics::init();

    // Qt graphics scene to show the pixmap
    QGraphicsScene scene;
    QGraphicsPixmapItem grarea(*(graphics::current_pixmap));
    scene.addItem( &grarea );

    // Qt graphics view of the pixmap world (scaled)
    QGraphicsView grview(&scene);
    grview.setCacheMode( QGraphicsView::CacheModeFlag::CacheNone );
    grview.setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    grview.scale(config::scale, config::scale);


    // timer to periodically update the graphics
    GrTimer myTimer( &grarea );
    QObject::connect(&app,SIGNAL(aboutToQuit()),&myTimer,SLOT(closing()));

    // new thread to periodically update the
    // simulation world
    //const size_t num_threads = std::thread::hardware_concurrency();
    //In the Linux virtual environment, the maximum number of threads was only 4

    const size_t num_threads = 20;
    const size_t rows_per_thread = config::height / num_threads;
    ThreadPool thread_pool(num_threads);

    std::thread update( [&]() {
        while(world::running) {
            std::this_thread::sleep_for( config::world_tick );
            {
                std::unique_lock<std::mutex> lock(world_mutex);
                for (size_t i = 0; i < num_threads; ++i) {
                    size_t start_row = i * rows_per_thread;
                    size_t end_row = (i == num_threads - 1) ? config::height : (i + 1) * rows_per_thread;
                    thread_pool.enqueue([=] {
                        world::next_generation(start_row, end_row);
                    });
                }
                std::cout << "Waiting for all tasks to finish" << std::endl;


                std::cout << "Total threads: " << thread_pool.get_total_threads() << std::endl;
                std::cout << "Active threads: " << thread_pool.get_active_threads() << std::endl;

                thread_pool.wait_all(); // wait for all tasks to finish
                std::cout << "Tasks completed" << std::endl;
                std::swap( world::current, world::next );
                world_updated = true;
            }
            world_cv.notify_one();
        };
    });

    grview.show();
    app.exec(); // main thread handles the Qt event loop
    update.join(); // collect update thread after app closes
    return EXIT_SUCCESS;
}
