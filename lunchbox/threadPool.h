/* Copyright (c) 2016, Mohamed-Ghaith Kaabi <mohamedghaith.kaabi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


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

/*!
 * Thread pool for tasks execution.
 * A task is a callable object taking no arguments and returing a value or void.
 * All the member mthods are thread safe.
 *
 * Example: @include tests/threadPool.cpp
 */

struct ThreadPool {

    /*!
     * \brief Construct a ThreadPool
     * \param size : number of threads in the thread pool
     */
    ThreadPool(size_t size = std::max(1u,std::thread::hardware_concurrency()))
        :   _stop(false)
    {
        for(size_t i = 0;i<size;++i)
            _threads.emplace_back([this]{this->work();});
    }


    /*! Destroy the thread pool.
     * This function will block until all the tasks a done
     */
    ~ThreadPool()
    {
        stop();
        _waitCondition.notify_all();
        joinAll();
    }

    /*!
     * \brief getSize
     * \return the number of threads used in the thread pool
     */
    size_t getSize()const
    {
        return _threads.size();

    }

    /*!
     * Post a new task in the thread pool.
     * \return a std::future containing the future result.
     */
    template<class F>
    auto post(F&& f) -> std::future<typename std::result_of<F()>::type>
    {
        checkStopped();
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

    /*!
     * Post a detached task in the thread pool.
     * The result of this task is not monitored.
     */
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

    /*!
     * Stop the thread pool. No task are accepted after calling this function.
     * If a task is posted after calling stop, a std::runtime_error is thrown
     */
    void stop()
    {
        _stop = true;
    }


    /*!
     * \return true is stop was called.
     */
    bool isStoped()const{
        return _stop;
    }


    /*!
     * \return true if there are pending tasks to be executed.
     */
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
