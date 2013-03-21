
/* Copyright (c) 2010-2013, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef LUNCHBOX_POOL_H
#define LUNCHBOX_POOL_H

#include <lunchbox/nonCopyable.h>  // base class
#include <lunchbox/scopedMutex.h>  // member
#include <lunchbox/spinLock.h>     // member
#include <lunchbox/thread.h>       // thread-safety checks

namespace lunchbox
{
    /** An object allocation pool. */
    template< typename T, bool locked = false > class Pool : public NonCopyable
    {
    public:
        /** Construct a new pool. @version 1.0 */
        Pool() : _lock( locked ? new SpinLock : 0 ) {}

        /** Destruct this pool. @version 1.0 */
        virtual ~Pool() { flush(); delete _lock; }

        /** @return a reusable or new item. @version 1.0 */
        T* alloc()
            {
                ScopedFastWrite mutex( _lock );
                LB_TS_SCOPED( _thread );
                if( _cache.empty( ))
                    return new T;

                T* item = _cache.back();
                _cache.pop_back();
                return item;
            }

        /** Release an item for reuse. @version 1.0 */
        void release( T* item )
            {
                ScopedFastWrite mutex( _lock );
                LB_TS_SCOPED( _thread );
                _cache.push_back( item );
            }

        /** Delete all cached items. @version 1.0 */
        void flush()
            {
                ScopedFastWrite mutex( _lock );
                LB_TS_SCOPED( _thread );
                while( !_cache.empty( ))
                {
                    delete _cache.back();
                    _cache.pop_back();
                }
            }

    private:
        SpinLock* const _lock;
        std::vector< T* > _cache;
        LB_TS_VAR( _thread );
    };
}
#endif // LUNCHBOX_POOL_H
