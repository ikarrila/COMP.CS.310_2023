#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH


#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);
    void wait_all();

    size_t get_total_threads();
    size_t get_active_threads();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    std::condition_variable tasks_cv;
    std::condition_variable completed_cv;
    int active_tasks = 0;
    bool stop;
};

#endif // THREAD_POOL_HH

