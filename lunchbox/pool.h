
/* Copyright (c) 2010-2017, Stefan Eilemann <eile@eyescale.ch>
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

#include <lunchbox/scopedMutex.h> // member
#include <lunchbox/spinLock.h>    // member
#include <lunchbox/thread.h>      // thread-safety checks

namespace lunchbox
{
/** A thread-safe object allocation pool. */
template <typename T>
class Pool : public boost::noncopyable
{
public:
    /** Construct a new pool. @version 1.0 */
    Pool() {}
    /** Destruct this pool. @version 1.0 */
    virtual ~Pool() { flush(); }
    /** @return a reusable or new item. @version 1.0 */
    T* alloc()
    {
        ScopedFastWrite mutex(_lock);
        if (_cache.empty())
            return new T;

        T* item = _cache.back();
        _cache.pop_back();
        return item;
    }

    /** Release an item for reuse. @version 1.0 */
    void release(T* item)
    {
        ScopedFastWrite mutex(_lock);
        _cache.push_back(item);
    }

    /** Delete all cached items. @version 1.0 */
    void flush()
    {
        ScopedFastWrite mutex(_lock);
        while (!_cache.empty())
        {
            delete _cache.back();
            _cache.pop_back();
        }
    }

private:
    SpinLock _lock;
    std::vector<T*> _cache;
};
}
#endif // LUNCHBOX_POOL_H
