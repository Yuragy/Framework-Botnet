#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads)
    : stop(false) {
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back(&ThreadPool::workerThread, this);
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

void ThreadPool::workerThread() {
    for (;;) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(this->queueMutex);
            this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
            if (this->stop && this->tasks.empty())
                return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
        }

        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "Exception in thread pool task: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in thread pool task" << std::endl;
        }
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
    using returnType = typename std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<returnType> res = task->get_future();
    {
        std::lock_guard<std::mutex> lock(queueMutex);

        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}
