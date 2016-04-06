
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <lunchbox/condition.h>   // used in inline method
#include <lunchbox/lock.h>        // used in inline method
#include <lunchbox/lockable.h>    // used in inline method
#include <lunchbox/types.h>

namespace lunchbox
{
class WriteOp;
class ReadOp;

/** @cond IGNORE */
template< class L, class T > struct ScopedMutexLocker {};
template< class L > struct ScopedMutexLocker< L, WriteOp >
{
    static inline void set( L& lock ) { lock.set(); }
    static inline void unset( L& lock ) { lock.unset(); }
};
template< class L > struct ScopedMutexLocker< L, ReadOp >
{
    static inline void set( L& lock ) { lock.setRead(); }
    static inline void unset( L& lock ) { lock.unsetRead(); }
};
template<> struct ScopedMutexLocker< Condition, WriteOp >
{
    static inline void set( Condition& cond ) { cond.lock(); }
    static inline void unset( Condition& cond ) { cond.unlock(); }
};
/** @endcond */

/**
 * A scoped mutex.
 *
 * The mutex is automatically set upon creation, and released when the scoped
 * mutex is destroyed, e.g., when the scope is left. The scoped mutex does
 * nothing if a 0 pointer for the lock is passed.
 * @deprecated Use boost::scoped_lock
 */
template< class L = Lock, class T = WriteOp >
class ScopedMutex
{
    typedef ScopedMutexLocker< L, T > LockTraits;

public:
    /**
     * Construct a new scoped mutex and set the given lock.
     *
     * Providing no Lock (0) is allowed, in which case the scoped mutex does
     * nothing.
     *
     * @param lock the mutex to set and unset, or 0.
     * @version 1.0
     */
    explicit ScopedMutex( L* lock ) : _lock( lock )
        { if( lock ) LockTraits::set( *lock ); }

    /** Construct a new scoped mutex and set the given lock. @version 1.0 */
    explicit ScopedMutex( L& lock ) : _lock( &lock )
        { LockTraits::set( lock ); }

    /** Move lock from rhs to new mutex. @version 1.5 */
    ScopedMutex( const ScopedMutex& rhs ) : _lock( rhs._lock )
    { const_cast< ScopedMutex& >( rhs )._lock = 0; }

    /** Move lock from rhs to this mutex. @version 1.5 */
    ScopedMutex& operator = ( ScopedMutex& rhs )
    {
        if( this != &rhs )
        {
            _lock = rhs._lock;
            rhs._lock = 0;
        }
        return *this;
    }

    /**
     * Construct a new scoped mutex for the given Lockable data structure.
     * @version 1.0
     */
    template< typename LB > explicit ScopedMutex( const LB& lockable )
    : _lock( &lockable.lock ) { LockTraits::set( lockable.lock ); }

    /** Destruct the scoped mutex and unset the mutex. @version 1.0 */
    ~ScopedMutex() { leave(); }

    /** Leave and unlock the mutex immediately. @version 1.0 */
    void leave() { if( _lock ) LockTraits::unset( *_lock ); _lock = 0; }

private:
    ScopedMutex();
    L* _lock;
};

/** A scoped mutex for a fast uncontended read operation. @version 1.1.2 */
typedef ScopedMutex< SpinLock, ReadOp > ScopedFastRead;

/** A scoped mutex for a fast uncontended write operation. @version 1.1.2 */
typedef ScopedMutex< SpinLock, WriteOp > ScopedFastWrite;

/** A scoped mutex for a read operation. @version 1.1.5 */
typedef ScopedMutex< Lock, ReadOp > ScopedRead;

/** A scoped mutex for a write operation. @version 1.1.5 */
typedef ScopedMutex< Lock, WriteOp > ScopedWrite;

/** A scoped mutex for a write operation on a condition. @version 1.3.6 */
typedef ScopedMutex< Condition, WriteOp > ScopedCondition;
}
#endif //LUNCHBOX_SCOPEDMUTEX_H
