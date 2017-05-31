
/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_SCOPEDMUTEX_H
#define LUNCHBOX_SCOPEDMUTEX_H

#include <lunchbox/lockable.h> // used in inline method
#include <lunchbox/types.h>
#include <mutex>

namespace lunchbox
{
/**
 * A scoped shared mutex.
 *
 * The mutex is automatically set upon creation, and released when the scoped
 * mutex is destroyed, e.g., when the scope is left. The scoped mutex does
 * nothing if a nullptr for the lock is passed.
 */
template <class L>
class UniqueSharedLock
{
public:
    explicit UniqueSharedLock(L& lock)
        : _lock(&lock)
    {
        lock.lock_shared();
    }

    explicit UniqueSharedLock(L* lock)
        : _lock(lock)
    {
        if (lock)
            lock->lock_shared();
    }

    template <typename LB>
    explicit UniqueSharedLock(const LB& lockable)
        : UniqueSharedLock(lockable.lock)
    {
    }

    /** Destruct the scoped mutex and unset the mutex. @version 1.0 */
    ~UniqueSharedLock()
    {
        if (_lock)
            _lock->unlock_shared();
    }

private:
    UniqueSharedLock() = delete;
    UniqueSharedLock(const UniqueSharedLock&) = delete;
    UniqueSharedLock(UniqueSharedLock&&) = delete;
    UniqueSharedLock& operator=(const UniqueSharedLock&) = delete;
    UniqueSharedLock& operator=(UniqueSharedLock&&) = delete;

    L* const _lock;
};

/**
 * A scoped mutex.
 *
 * The mutex is automatically set upon creation, and released when the scoped
 * mutex is destroyed, e.g., when the scope is left. The scoped mutex does
 * nothing if a nullptr for the lock is passed.
 */
template <class L>
class UniqueLock : public std::unique_lock<L>
{
public:
    UniqueLock(L* lock_)
        : std::unique_lock<L>(lock_ ? std::unique_lock<L>(*lock_)
                                    : std::unique_lock<L>())
    {
    }

    UniqueLock(L& lock_)
        : std::unique_lock<L>(lock_)
    {
    }

    template <typename LB>
    explicit UniqueLock(const LB& lockable)
        : UniqueLock(lockable.lock)
    {
    }

private:
    UniqueLock() = delete;
    UniqueLock(const UniqueLock&) = delete;
    UniqueLock& operator=(const UniqueLock&) = delete;
};

/** A scoped mutex for a fast uncontended read operation. @version 1.1.2 */
using ScopedFastRead = UniqueSharedLock<SpinLock>;

/** A scoped mutex for a fast uncontended write operation. @version 1.1.2 */
using ScopedFastWrite = UniqueLock<SpinLock>;

/** A scoped mutex for a read operation. @version 1.1.5 */
using ScopedRead = UniqueLock<std::mutex>;

/** A scoped mutex for a write operation. @version 1.1.5 */
using ScopedWrite = UniqueLock<std::mutex>;
}
#endif // LUNCHBOX_SCOPEDMUTEX_H
