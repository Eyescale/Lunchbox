
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_LOCK_H
#define LUNCHBOX_LOCK_H

#include <lunchbox/api.h>
#include <lunchbox/compiler.h>
#include <boost/noncopyable.hpp>

namespace lunchbox
{
namespace detail { class Lock; }

/**
 * A lock (mutex) primitive.
 * @sa ScopedMutex
 * @deprecated Use boost::mutex
 *
 * Example: @include tests/perf/lock.cpp
 */
class Lock : public boost::noncopyable
{
public:
    /** Construct a new lock. @version 1.0 */
    LUNCHBOX_API Lock();

    /** Destruct the lock. @version 1.0 */
    LUNCHBOX_API ~Lock();

    /** Acquire the lock. @version 1.0 */
    LUNCHBOX_API void set();

    /** Release the lock. @version 1.0 */
    LUNCHBOX_API void unset();

    /** Acquire the lock shared with other readers. @version 1.3.2 */
    void setRead() { set(); }

    /** Release a shared read lock. @version 1.3.2 */
    void unsetRead() { unset(); }

    /**
     * Attempt to acquire the lock.
     *
     * This method implements an atomic test-and-set operation.
     *
     * @return <code>true</code> if the lock was set, <code>false</code> if
     *         it was not set.
     * @version 1.0
     */
    LUNCHBOX_API bool trySet();

    /**
     * Test if the lock is set.
     *
     * @return <code>true</code> if the lock is set, <code>false</code> if
     *         it is not set.
     * @version 1.0
     */
    LUNCHBOX_API bool isSet();

private:
    detail::Lock* const _impl;
};// LB_DEPRECATED;
}
#endif //LUNCHBOX_LOCK_H
