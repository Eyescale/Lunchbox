#ifndef LUNCHBOX_THREADPOOL_H
#define LUNCHBOX_THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


namespace lunchbox {


struct ThreadPool {

    ThreadPool(size_t size = std::max(1u,std::thread::hardware_concurrency()))
        :   _stop(false)
    {
        for(size_t i = 0;i<size;++i)
            _threads.emplace_back([this]{this->work();});
    }

    ~ThreadPool()
    {
        stop();
        _waitCondition.notify_all();
        joinAll();
    }

    size_t getSize()const
    {
        return _threads.size();

    }

    template<class F>
    auto post(F&& f) -> std::future<typename std::result_of<F()>::type>
    {
        checkStopped();

        if(_stop) throw std::runtime_error("posting on stopped thread pool");

        using ReturnType = typename std::result_of<F()>::type;

        auto task  = std::make_shared<std::packaged_task<ReturnType()>>(
                        std::forward<F>(f)
                     );

        std::future<ReturnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _tasks.emplace([task](){ (*task)();});
        }
        _waitCondition.notify_one();
        return res;
    }

    template<class F>
    void postDetached(F&& f)
    {
        checkStopped();
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _tasks.emplace(f);
        }
        _waitCondition.notify_one();
    }

    void stop()
    {
        _stop = true;
    }

    bool hasPendingJobs()const
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        return _tasks.size();
    }

protected:

    void checkStopped(){
        if(_stop) throw std::runtime_error("posting on stopped thread pool");

    }

    void joinAll()
    {
        for(std::thread &worker: _threads) worker.join();
    }

    void work()
    {
        for(;;)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                _waitCondition.wait(lock,
                    [this]{ return _stop || !_tasks.empty(); });
                if(_stop && _tasks.empty())
                    return;
                task = std::move(_tasks.front());
                _tasks.pop();
            }
            task();
        }
    }

private:
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _tasks;
    mutable std::mutex _queueMutex;
    std::condition_variable _waitCondition;
    bool _stop;
};



}


#endif //LUNCHBOX_THREADPOOL_H
