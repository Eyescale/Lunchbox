
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@eyescale.ch>
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

#include "condition.h"
#include "debug.h"
#include "time.h"

#include <cstring>
#include <errno.h>

#ifdef _WIN32
#include "condition_w32.ipp"
#else
#include <pthread.h>
#include <sys/time.h>
#endif

namespace lunchbox
{
namespace detail
{
class Condition
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};
}

Condition::Condition()
    : _impl(new detail::Condition)
{
    // mutex init
    int error = pthread_mutex_init(&_impl->mutex, 0);
    if (error)
    {
        LBERROR << "Error creating pthread mutex: " << strerror(error)
                << std::endl;
        return;
    }

    // condvar init
    error = pthread_cond_init(&_impl->cond, 0);
    if (error)
    {
        LBERROR << "Error creating pthread condition: " << strerror(error)
                << std::endl;
        return;
    }
}

Condition::~Condition()
{
    int error = pthread_mutex_destroy(&_impl->mutex);
    if (error)
        LBERROR << "Error destroying pthread mutex: " << strerror(error)
                << " at " << backtrace << std::endl;

    error = pthread_cond_destroy(&_impl->cond);
    if (error)
        LBERROR << "Error destroying pthread condition: " << strerror(error)
                << std::endl;

    delete _impl;
}

void Condition::lock()
{
    pthread_mutex_lock(&_impl->mutex);
}

void Condition::signal()
{
    pthread_cond_signal(&_impl->cond);
}

void Condition::broadcast()
{
    pthread_cond_broadcast(&_impl->cond);
}

void Condition::unlock()
{
    pthread_mutex_unlock(&_impl->mutex);
}

void Condition::wait()
{
    pthread_cond_wait(&_impl->cond, &_impl->mutex);
}

bool Condition::timedWait(const uint32_t timeout)
{
    if (timeout == LB_TIMEOUT_INDEFINITE)
    {
        wait();
        return true;
    }

    const uint32_t time =
        timeout == LB_TIMEOUT_DEFAULT ? 300000 /* 5 min */ : timeout;

#ifdef _WIN32
    int error =
        pthread_cond_timedwait_w32_np(&_impl->cond, &_impl->mutex, time);
#else
    const timespec delta = convertToTimespec(time);
    timeval now;
    gettimeofday(&now, 0);

    timespec then;
    then.tv_sec = delta.tv_sec + now.tv_sec;
    then.tv_nsec = delta.tv_nsec + now.tv_usec * 1000;
    while (then.tv_nsec > 1000000000)
    {
        ++then.tv_sec;
        then.tv_nsec -= 1000000000;
    }

    int error = pthread_cond_timedwait(&_impl->cond, &_impl->mutex, &then);
#endif
    if (error == ETIMEDOUT)
        return false;

    if (error)
        LBERROR << "pthread_cond_timedwait failed: " << strerror(error)
                << std::endl;
    return true;
}
}
