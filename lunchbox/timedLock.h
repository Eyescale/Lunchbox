
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_TIMEDLOCK_H
#define LUNCHBOX_TIMEDLOCK_H

#include <boost/noncopyable.hpp>
#include <lunchbox/api.h>
#include <lunchbox/types.h>

namespace lunchbox
{
namespace detail
{
class TimedLock;
}

/**
 * A mutex with timeout capabilities.
 * @deprecated Use boost::timed_lock
 *
 * Example: @include tests/timedLock.cpp
 */
class TimedLock : public boost::noncopyable
{
public:
    /** Construct a new timed lock. @version 1.0 */
    LUNCHBOX_API TimedLock();

    /** Destruct the lock. @version 1.0 */
    LUNCHBOX_API ~TimedLock();

    /**
     * Set the lock.
     *
     * @param timeout the timeout in milliseconds to wait for the lock,
     *                or LB_TIMEOUT_INDEFINITE to wait indefinitely.
     * @return true if the lock was acquired, false if not.
     * @version 1.0
     */
    LUNCHBOX_API bool set(const uint32_t timeout = LB_TIMEOUT_INDEFINITE);

    /** Release the lock. @version 1.0 */
    LUNCHBOX_API void unset();

    /**
     * Attempt to set the lock.
     *
     * @return true if the lock was acquired, false if not.
     * @version 1.0
     */
    LUNCHBOX_API bool trySet();

    /**
     * Test if the lock is set.
     *
     * @return true if the lock is set, false if it is not set.
     * @version 1.0
     */
    LUNCHBOX_API bool isSet();

private:
    detail::TimedLock* const _impl;
};
}
#endif // LUNCHBOX_TIMEDLOCK_H
