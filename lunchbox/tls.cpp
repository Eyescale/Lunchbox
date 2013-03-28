
/* Copyright (c) 2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "tls.h"
#include "debug.h"
#include <cstring>
#include <pthread.h>

namespace lunchbox
{
namespace detail
{
class TLS
{
public:
    TLS( lunchbox::TLS::ThreadDestructor_t df ) : dtorFunc ( df ) {}

    pthread_key_t key;
    const lunchbox::TLS::ThreadDestructor_t dtorFunc;
};
}

TLS::TLS( ThreadDestructor_t dtorFunc )
    : impl_( new detail::TLS( dtorFunc ))
{
    const int error = pthread_key_create( &impl_->key, dtorFunc );
    if( error )
    {
        LBERROR << "Can't create thread-specific key: "
                << strerror( error ) << std::endl;
        LBASSERT( !error );
    }
}

TLS::~TLS()
{
    void* data = get();
    if( data && impl_->dtorFunc )
        impl_->dtorFunc( data );

    pthread_key_delete( impl_->key );
    delete impl_;
}

void TLS::set( const void* data )
{
    pthread_setspecific( impl_->key, data );
}

void* TLS::get()
{
    return pthread_getspecific( impl_->key );
}

const void* TLS::get() const
{
    return pthread_getspecific( impl_->key );
}

}
