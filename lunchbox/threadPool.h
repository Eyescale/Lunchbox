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

#ifndef LUNCHBOX_THREADPOOL_H
#define LUNCHBOX_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

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
    /**
     * Construct a ThreadPool
     * @param size number of threads in the thread pool
     */
    inline ThreadPool(
        const size_t size = std::max(1u, std::thread::hardware_concurrency()));
    /**
     * Destroy this thread pool.
     * Will block until all the tasks are done.
     */
    inline ~ThreadPool();

    /**
     * @return the number of threads used in the thread pool
     */
    inline size_t getSize() const;

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

    /**
     * @return true if there are pending tasks to be executed.
     */
    inline bool hasPendingJobs() const;

private:
    inline void joinAll();
    inline void work();

    std::vector<std::thread> _threads;
    std::queue<std::function<void()> > _tasks;
    mutable std::mutex _tasksMutex;
    std::condition_variable _waitCondition;
    bool _stop;
};
}

#include "threadPool.ipp"

#endif // LUNCHBOX_THREADPOOL_H
