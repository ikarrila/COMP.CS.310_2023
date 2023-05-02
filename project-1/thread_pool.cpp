#include "thread_pool.hh"

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->tasks_mutex);
                    this->tasks_cv.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });

                    if (this->stop && this->tasks.empty()) {
                        return;
                    }

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    this->active_tasks++;
                }

                task();

                {
                    std::unique_lock<std::mutex> lock(this->tasks_mutex);
                    this->active_tasks--;
                    this->completed_cv.notify_all();
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        stop = true;
    }
    tasks_cv.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(tasks_mutex);

        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace(std::move(task));
    }
    tasks_cv.notify_one();
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(tasks_mutex);
    completed_cv.wait(lock, [this] {
        return active_tasks == 0;
    });
}

size_t ThreadPool::get_total_threads() {
    return workers.size();
}

size_t ThreadPool::get_active_threads() {
    std::unique_lock<std::mutex> lock(tasks_mutex);
    return active_tasks;
}
