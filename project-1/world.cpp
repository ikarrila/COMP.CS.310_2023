#include "world.hh"
#include "stopwatch.hh"

#include "tuni.hh"
TUNI_WARN_OFF()
#include <QtGlobal>
#include <QDebug>
TUNI_WARN_ON()

#include <random>
#include <iostream>
#include <string>
#include <execution>
#include <thread>

#include "thread_pool.hh"

namespace world {

// actual variables (extern in header file):
std::unique_ptr< world_t > current;
std::unique_ptr< world_t > next;

bool running = true;

namespace {
    // QtCreator might give non-pod warning here, explanation:
    // https://github.com/KDE/clazy/blob/master/docs/checks/README-non-pod-global-static.md
    auto rand_generator = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());

    // glider data is from: https://raw.githubusercontent.com/EsriCanada/gosper_glider_gun/master/gosper.txt
    std::string glider_gun[] = {
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 1 0 1 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 1 0 0 0 0 0 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 1 0 0 1 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 1 0 0 0 1 1 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 1 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 1 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 1 1 1 1 1 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
        "0000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
    };
}

void init(void)
{
    // creaate and fill empty world data
    current = std::make_unique<world_t>();
    current->fill(Block::empty);
    next = std::make_unique<world_t>();
    next->fill(Block::empty);

    // create a random starting world
    config::coord_t x = 0;
    config::coord_t y = 0;
    for( x = 0; x < config::width; ++x )
    {
        for( y = 0; y < config::height; ++y )
        {
            if( x < (config::width/2) and y < (config::height/2) )
                continue; // leave space for the glider gun

            bool b = rand_generator();
            if(b) {
                (*current)[xy2array(x,y)] = Block::empty;
            } else {
                (*current)[xy2array(x,y)] = Block::occupied;
            }

        }
    }


    // glider gun data to world data
    const auto glider_pos = 20;
    x = glider_pos;
    y = glider_pos;
    for(auto& line : glider_gun)
    {
        for(char c : line)
        {
            if(c == '1') {
                (*current)[xy2array(x,y)] = Block::occupied;
                ++x;
            } else if(c == '0') {
                (*current)[xy2array(x,y)] = Block::empty;
                ++x;
            } else {
                // all other chars ignored
            }
        }
        x = glider_pos;
        ++y;
    }
}

/* OLD VERSION
void next_generation(void)
{
    stopwatch clock;

    for(config::coord_t x = 0; x < config::width; x++ ) {
        for(config::coord_t y = 0; y < config::height; y++) {
            if( world::running == false) return;  // nothing done if simulation is not running

            auto current_pos = xy2array(x,y);
            auto neighbours = num_neighbours(x,y);

            // https://en.wikipedia.org/wiki/Conway's_Game_of_Life#Rules
            if( (*current)[ current_pos ] == Block::occupied && (neighbours==2 || neighbours==3) ) {
                // 1. stay alive if 2 or 3 neighbours
                (*next)[ current_pos ] = Block::occupied;
            } else if( (*current)[ current_pos ] == Block::empty && neighbours == 3 ) {
                // 2. spawn new if exactly three neighbours
                (*next)[ current_pos ] = Block::occupied;
            } else {
                // 3. else cell will be empty
                (*next)[ current_pos ] = Block::empty;
            }

        }
    }

    std::cerr << "next world created in: " << clock.elapsed() << std::endl;

    std::swap( current, next );
}
*/
void next_generation(size_t start_row, size_t end_row) {
    for (size_t row = start_row; row < end_row; ++row) {
        for (size_t col = 0; col < config::width; ++col) {
            size_t neighbors = num_neighbours(col, row);
            bool alive = (*current)[xy2array(col, row)] == Block::occupied;

            if (alive && (neighbors < 2 || neighbors > 3)) {
                (*next)[xy2array(col, row)] = Block::empty;
            } else if (!alive && neighbors == 3) {
                (*next)[xy2array(col, row)] = Block::occupied;
            } else {
                (*next)[xy2array(col, row)] = (*current)[xy2array(col, row)];
            }
        }
    }
}

void update_concurrent() {
    //const size_t num_threads = std::thread::hardware_concurrency();
    const size_t num_threads = 10;
    const size_t rows_per_thread = config::height / num_threads;
    ThreadPool thread_pool(num_threads);

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
    std::swap( current, next );
}

} // world
