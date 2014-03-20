
/* Copyright (c) 2013-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_TLS_H
#define LUNCHBOX_TLS_H

#include <lunchbox/api.h>
#include <boost/noncopyable.hpp>

namespace lunchbox
{
namespace detail { class TLS; }

/** Provides thread-local storage API used by PerThread and PerThreadRef. */
class TLS : public boost::noncopyable
{
public:
    typedef void (*ThreadDestructor_t)(void*);

    /**
     * Construct a new per-thread storage.
     *
     * @param dtorFunc the destructor function called to destroy thread-local
     *                 storage, not called if 0.
     * @version 1.7.2
     */
    LUNCHBOX_API explicit TLS( ThreadDestructor_t dtorFunc );

    /** Destruct the per-thread storage. @version 1.7.2 */
    LUNCHBOX_API ~TLS();

    /** Set the data for this thread-local storage. @version 1.7.2 */
    LUNCHBOX_API void set( const void* data );

    /** @return the data for this thread-local storage. @version 1.7.2 */
    LUNCHBOX_API void* get();

    /** @return the data for this thread-local storage. @version 1.7.2 */
    LUNCHBOX_API const void* get() const;

private:
    detail::TLS* const impl_;
};
}

#endif //LUNCHBOX_TLS_H
