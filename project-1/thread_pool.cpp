#include "thread_pool.hh"

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    // Iterate through the number of requested threads
    for (size_t i = 0; i < num_threads; ++i) {
        // Create a new thread, with a lambda function that captures 'this' pointer of ThreadPool object
        workers.emplace_back([this] {
            // This loop continues until 'stop' is set to true and there are no more tasks in the queue
            while (true) {
                std::function<void()> task;

                // Lock the tasks_mutex to access the tasks queue
                {
                    std::unique_lock<std::mutex> lock(this->tasks_mutex);
                    // Wait for a new task to be added to the queue, or for the 'stop' flag to be set
                    this->tasks_cv.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });

                    // If 'stop' is set and there are no more tasks, break out of the loop
                    if (this->stop && this->tasks.empty()) {
                        return;
                    }

                    // Retrieve the next task from the queue and remove it
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    // Increment the count of active tasks
                    this->active_tasks++;
                }

                // Execute the task
                task();

                // Lock the tasks_mutex again to update the active_tasks count and notify 'completed_cv'
                {
                    std::unique_lock<std::mutex> lock(this->tasks_mutex);
                    // Decrement the count of active tasks
                    this->active_tasks--;
                    // Notify all waiting threads that a task has completed
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
