
/* Copyright (c) 2010-2017, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_SPINLOCK_H
#define LUNCHBOX_SPINLOCK_H

#include <lunchbox/atomic.h> // member
#include <lunchbox/thread.h> // used in inline method

#include <memory>

namespace lunchbox
{
/**
 * A fast lock for uncontended memory access.
 *
 * If Thread::yield() does not work, priority inversion is possible. If used as
 * a read-write lock, readers or writers will starve on high contention.
 *
 * @sa ScopedMutex
 *
 * Example: @include tests/perf/lock.cpp
 */
class SpinLock : public boost::noncopyable
{
public:
    /** Construct a new lock. @version 1.0 */
    LUNCHBOX_API SpinLock();

    /** Destruct the lock. @version 1.0 */
    LUNCHBOX_API ~SpinLock();

    /** Acquire the lock exclusively. @version 1.0 */
    LUNCHBOX_API void set();
    void lock() { set(); }
    /** Release an exclusive lock. @version 1.0 */
    LUNCHBOX_API void unset();
    void unlock() { unset(); }
    /**
     * Attempt to acquire the lock exclusively.
     *
     * @return true if the lock was set, false if it was not set.
     * @version 1.0
     */
    LUNCHBOX_API bool trySet();

    /** Acquire the lock shared with other readers. @version 1.1.2 */
    LUNCHBOX_API void setRead();
    void lock_shared() { setRead(); }
    /** Release a shared read lock. @version 1.1.2 */
    LUNCHBOX_API void unsetRead();
    void unlock_shared() { unsetRead(); }
    /**
     * Attempt to acquire the lock shared with other readers.
     *
     * @return true if the lock was set, false if it was not set.
     * @version 1.1.2
     */
    LUNCHBOX_API bool trySetRead();

    /**
     * Test if the lock is set.
     *
     * @return true if the lock is set, false if it is not set.
     * @version 1.0
     */
    LUNCHBOX_API bool isSet();

    /**
     * Test if the lock is set exclusively.
     *
     * @return true if the lock is set, false if it is not set.
     * @version 1.1.2
     */
    LUNCHBOX_API bool isSetWrite();

    /**
     * Test if the lock is set shared.
     *
     * @return true if the lock is set, false if it is not set.
     * @version 1.1.2
     */
    LUNCHBOX_API bool isSetRead();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;
};
}
#endif // LUNCHBOX_SPINLOCK_H
