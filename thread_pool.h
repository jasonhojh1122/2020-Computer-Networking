#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <iostream>

namespace tp {

template<typename T>
class ThreadsafeQueue {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    ThreadsafeQueue() {}
    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = std::move(data_queue.front());
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        return res;
    }
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T> ();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool empty() {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

class ThreadPool {
private:
    std::atomic_bool done;
    ThreadsafeQueue<std::function<void()>> work_queue;
    std::vector<std::thread> workers;
    void work() {
        while(!done) {
            std::function<void()> task;
            if (work_queue.try_pop(task)) {
                task();
            }
            else {
                std::this_thread::yield();
            }
        }
    }

public:
    ThreadPool() : done(false) {
        unsigned const int thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned int i = 0; i < thread_count; ++i) {
                workers.push_back(std::thread(&ThreadPool::work, this));
            }
            std::cout << "Number of threads in thread pool: " << thread_count << "\n";
        }
        catch(...) {
            done = true;
            throw;
        }
    }
    
    ~ThreadPool() {
        done = true;
        for (unsigned int i = 0; i < workers.size(); ++i) {
            if (workers[i].joinable())
                workers[i].join();
        }
    }

    template<typename FunctionType, typename... Args>
    void submit(FunctionType f, Args&&... args) {
        work_queue.push(std::bind(f, std::forward<Args>(args)...));
    }
};

}