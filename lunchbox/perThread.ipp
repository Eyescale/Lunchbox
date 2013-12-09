
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

namespace lunchbox
{
template< class T, void (*D)( T* ) > PerThread<T, D>::PerThread()
    : tls_( (TLS::ThreadDestructor_t)D )
{}

template< class T, void (*D)( T* ) > PerThread<T, D>::~PerThread()
{}

template< class T, void (*D)( T* ) >
PerThread<T, D>& PerThread<T, D>::operator = ( const T* data )
{
    tls_.set( static_cast< const void* >( data ));
    return *this;
}

template< class T, void (*D)( T* ) >
PerThread<T, D>& PerThread<T, D>::operator = ( const PerThread<T, D>& rhs )
{
    tls_.set( rhs.tls_.get( ));
    return *this;
}

template< class T, void (*D)( T* ) > T* PerThread<T, D>::get()
{
    return static_cast< T* >( tls_.get( ));
}

template< class T, void (*D)( T* ) > const T* PerThread<T, D>::get() const
{
    return static_cast< const T* >( tls_.get( ));
}

template< class T, void (*D)( T* ) > T* PerThread<T, D>::operator->()
{
    return static_cast< T* >( tls_.get( ));
}

template< class T, void (*D)( T* ) >
const T* PerThread<T, D>::operator->() const
{
    return static_cast< T* >( tls_.get( ));
}

template< class T, void (*D)( T* ) > bool PerThread<T, D>::operator ! () const
{
    return tls_.get() == 0;
}

template< class T, void (*D)( T* ) > bool PerThread<T, D>::isValid() const
{
    return tls_.get() != 0;
}
}
