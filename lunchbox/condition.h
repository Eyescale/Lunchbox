
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef LUNCHBOX_CONDITION_H
#define LUNCHBOX_CONDITION_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>

namespace lunchbox
{
namespace detail { class Condition; }

    /**
     * A condition variable and associated lock.
     * Follows closely pthread_condition and mutex
     */
    class Condition
    {
    public:
        /** Construct a new condition variable. @version 1.0 */
        LUNCHBOX_API Condition();

        /** Destruct this condition variable. @version 1.0 */
        LUNCHBOX_API ~Condition();

        /** Lock the mutex. @version 1.0 */
        LUNCHBOX_API void lock();

        /** Unlock the mutex. @version 1.0 */
        LUNCHBOX_API void unlock();

        /** Signal the condition. @version 1.0 */
        LUNCHBOX_API void signal();

        /** Broadcast the condition. @version 1.0 */
        LUNCHBOX_API void broadcast();

        /**
         * Atomically unlock the mutex, wait for a signal and relock the mutex.
         * @version 1.0
         */
        LUNCHBOX_API void wait();

        /**
         * Atomically unlock the mutex, wait for a signal and relock the mutex.
         *
         * The operation is aborted after the given timeout and false is
         * returned.
         *
         * @param timeout the timeout in milliseconds to wait for the signal.
         * @return true on success, false on timeout.
         * @version 1.0
         */
        LUNCHBOX_API bool timedWait( const uint32_t timeout );

    private:
        detail::Condition* const _impl;
    };
}

#endif //LUNCHBOX_CONDITION_H
