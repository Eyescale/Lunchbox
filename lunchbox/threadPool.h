/* Copyright (c) 2016-2017, Mohamed-Ghaith Kaabi <mohamedghaith.kaabi@gmail.com>
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

#pragma once

#include <lunchbox/api.h>

#include <condition_variable> // member
#include <functional>
#include <future> // inline return value
#include <queue>  // member
#include <thread> // member
#include <vector> // member

namespace lunchbox
{
/**
 * Thread pool for tasks execution.
 * A task is a callable object taking no arguments and returing a value or void.
 * All the member methods are thread safe.
 *
 * Example: @include tests/threadPool.cpp
 */
class ThreadPool
{
public:
    /** @return the application-global thread pool. */
    static LUNCHBOX_API ThreadPool& getInstance();

    /**
     * Construct a new ThreadPool.
     *
     * @param size number of threads in the thread pool
     * @sa getInstance() for the recommended thread pool.
     */
    LUNCHBOX_API ThreadPool(const size_t size);
    /**
     * Destroy this thread pool.
     * Will block until all the tasks are done.
     */
    LUNCHBOX_API ~ThreadPool();

    /**
     * @return the number of threads used in the thread pool
     */
    LUNCHBOX_API size_t getSize() const;

    /**
     * Post a new task in the thread pool.
     * @return a std::future containing the future result.
     */
    template <typename F>
    inline std::future<typename std::result_of<F()>::type> post(F&& f);

    /**
     * Post a detached task in the thread pool.
     * The result of this task is not monitored.
     */
    template <typename F>
    inline void postDetached(F&& f);

    /** @return true if there are pending tasks to be executed. */
    LUNCHBOX_API bool hasPendingJobs() const;

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    LUNCHBOX_API void joinAll();
    LUNCHBOX_API void work();

    std::vector<std::thread> _threads;
    std::queue<std::function<void()> > _tasks;
    mutable std::mutex _mutex;
    std::condition_variable _condition;
    bool _stop;
};

template <typename F>
std::future<typename std::result_of<F()>::type> ThreadPool::post(F&& f)
{
    using ReturnType = typename std::result_of<F()>::type;

    auto task =
        std::make_shared<std::packaged_task<ReturnType()> >(std::forward<F>(f));

    auto res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _tasks.emplace([task]() { (*task)(); });
    }
    _condition.notify_one();
    return res;
}

template <typename F>
void ThreadPool::postDetached(F&& f)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _tasks.emplace(f);
    }
    _condition.notify_one();
}
}
