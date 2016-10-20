
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
template< class T > Buffer< T >::Buffer( const Buffer< T >& from )
{
    *this = from;
}

template< class T > Buffer< T >::Buffer( Buffer< T >&& from )
{
    std::swap( _data, from._data );
    std::swap( _size, from._size );
}

template< class T > T* Buffer< T >::pack()
{
    if( _maxSize != _size )
    {
        _data = static_cast< T* >( realloc( _data, _size * sizeof( T )));
        _maxSize = _size;
    }
    return _data;
}

template< class T >
Buffer< T >& Buffer< T >::operator = ( const Buffer< T >& from )
{
    if( this != &from )
        replace( from );
    return *this;
}

template< class T >
Buffer< T >& Buffer< T >::operator = ( Buffer< T >&& from )
{
    if( this == &from )
        return *this;
    std::swap( _data, from._data );
    std::swap( _size, from._size );
    return *this;
}

template< class T > T* Buffer< T >::resize( const uint64_t newSize )
{
    _size = newSize;
    if( newSize <= _maxSize )
        return _data;

    // avoid excessive reallocs
    const uint64_t nElems = newSize + (newSize >> 3);
    const uint64_t nBytes = nElems * sizeof( T );
    _data = static_cast< T* >( realloc( _data, nBytes ));
    _maxSize = nElems;
    return _data;
}

template< class T > void Buffer< T >::grow( const uint64_t newSize )
{
    if( newSize > _size )
        resize( newSize );
}

template< class T > T* Buffer< T >::reserve( const uint64_t newSize )
{
    if( newSize <= _maxSize )
        return _data;

    _data = static_cast< T* >( realloc( _data, newSize * sizeof( T )));
    _maxSize = newSize;
    return _data;
}

template< class T > T* Buffer< T >::reset( const uint64_t newSize )
{
    reserve( newSize );
    setSize( newSize );
    return _data;
}

template< class T >
void Buffer< T >::append( const T* data, const uint64_t size )
{
    LBASSERT( data );
    LBASSERT( size );

    const uint64_t oldSize = _size;
    resize( oldSize + size );
    memcpy( _data + oldSize, data, size * sizeof( T ));
}

template< class T > void Buffer< T >::append( const T& element )
{
    append( &element, 1 );
}

template< class T >
void Buffer< T >::replace( const void* data, const uint64_t size )
{
    LBASSERT( data );
    LBASSERT( size );

    reserve( size );
    memcpy( _data, data, size * sizeof( T ));
    _size = size;
}

template< class T > void Buffer< T >::swap( Buffer< T >& buffer )
{
    T*             tmpData    = buffer._data;
    const uint64_t tmpSize    = buffer._size;
    const uint64_t tmpMaxSize = buffer._maxSize;

    buffer._data = _data;
    buffer._size = _size;
    buffer._maxSize = _maxSize;

    _data     = tmpData;
    _size     = tmpSize;
    _maxSize = tmpMaxSize;
}

template< class T > bool Buffer< T >::setSize( const uint64_t size )
{
    LBASSERT( size <= _maxSize );
    if( size > _maxSize )
        return false;

    _size = size;
    return true;
}
}
