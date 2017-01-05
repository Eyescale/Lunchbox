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

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>

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
    ThreadPool( const size_t size =
                    std::max( 1u, std::thread::hardware_concurrency( )))
        : _stop( false )
    {
        for( size_t i = 0; i < size; ++i )
            _threads.emplace_back( [this]{ this->work(); });
    }

    /**
     * Destroy this thread pool.
     * Will block until all the tasks are done.
     */
    ~ThreadPool()
    {
        _stop = true;
        _waitCondition.notify_all();
        joinAll();
    }

    /**
     * @return the number of threads used in the thread pool
     */
    size_t getSize() const
    {
        return _threads.size();
    }

    /**
     * Post a new task in the thread pool.
     * @return a std::future containing the future result.
     */
    template< class F >
    std::future< typename std::result_of< F( )>::type > post( F&& f )
    {
        using ReturnType = typename std::result_of< F( )>::type;

        auto task = std::make_shared< std::packaged_task< ReturnType( )>>(
                        std::forward< F >( f ));

        std::future< ReturnType > res = task->get_future();
        {
            std::unique_lock< std::mutex > lock( _tasksMutex );
            _tasks.emplace([task](){ (*task)(); });
        }
        _waitCondition.notify_one();
        return res;
    }

    /**
     * Post a detached task in the thread pool.
     * The result of this task is not monitored.
     */
    template< class F > void postDetached(F&& f)
    {
        {
            std::unique_lock< std::mutex > lock( _tasksMutex );
            _tasks.emplace(f);
        }
        _waitCondition.notify_one();
    }

    /**
     * @return true if there are pending tasks to be executed.
     */

    bool hasPendingJobs() const
    {
        std::unique_lock< std::mutex > lock( _tasksMutex );
        return !_tasks.empty();
    }

private:
    void joinAll()
    {
        for( auto& thread: _threads )
            thread.join();
    }

    void work()
    {
        for(;;)
        {
            std::function< void( )> task;
            {
                std::unique_lock< std::mutex > lock( _tasksMutex );
                _waitCondition.wait( lock, [this]
                                           {
                                               return _stop || !_tasks.empty();
                                           });
                if( _stop )
                    return;
                task = std::move( _tasks.front( ));
                _tasks.pop();
            }
            task();
        }
    }


    std::vector< std::thread > _threads;
    std::queue< std::function< void( )>> _tasks;
    mutable std::mutex _tasksMutex;
    std::condition_variable _waitCondition;
    bool _stop;
};

}

#endif //LUNCHBOX_THREADPOOL_H
