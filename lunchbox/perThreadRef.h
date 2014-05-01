
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_PERTHREADREF_H
#define LUNCHBOX_PERTHREADREF_H

#include <lunchbox/tls.h> // member

namespace lunchbox
{

/** Thread-specific storage for a RefPtr. */
template< typename T > class PerThreadRef : public boost::noncopyable
{
public:
    /** Construct a new per-thread RefPtr. @version 1.0 */
    PerThreadRef();
    /** Destruct a per-thread RefPtr. @version 1.0 */
    ~PerThreadRef();

    /** Assign a RefPtr to the thread-local storage. @version 1.0 */
    PerThreadRef<T>& operator = ( RefPtr< T > data );

    /** Assign a RefPtr to the thread-local storage. @version 1.0 */
    PerThreadRef<T>& operator = ( const PerThreadRef<T>& rhs );

    /** @return the RefPtr from the thread-local storage. @version 1.0 */
    RefPtr< const T > get() const;
    /** @return the RefPtr from the thread-local storage. @version 1.0 */
    RefPtr< T > get();

    /**
     * @return the C pointer of the RefPtr from the thread-local storage.
     * @version 1.0
     */
    T* getPointer();

    /**
     * @return the object held by the RefPtr in the thread-local storage.
     * @version 1.0
     */
    T* operator->();

    /**
     * @return the object held by the RefPtr in the thread-local storage.
     * @version 1.0
     */
    const T* operator->() const;

    /**
     * @return true if the two objects hold the same C pointer.
     * @version 1.0
     */
    bool operator == ( const PerThreadRef& rhs ) const
        { return ( get() == rhs.get( )); }

    /**
     * @return true if the two objects hold the same C pointer.
     * @version 1.0
     */
    bool operator == ( const RefPtr< T >& rhs ) const
        { return ( get()==rhs ); }

    /**
     * @return true if the two objects hold the same C pointer.
     * @version 1.0
     */
    bool operator != ( const RefPtr< T >& rhs ) const
        { return ( get()!=rhs ); }

    /**
     * @return true if the thread-local storage holds a 0 pointer.
     * @version 1.0
     */
    bool operator ! () const;

    /**
     * @return true if the thread-local storage holds a non-0 pointer.
     * @version 1.0
     */
    bool isValid() const;

private:
    TLS tls_;
};

template< typename T > PerThreadRef<T>::PerThreadRef()
    : tls_( 0 )
{}

template< typename T > PerThreadRef<T>::~PerThreadRef()
{
    RefPtr< T > object = get();
    object.unref();
}

template< typename T >
PerThreadRef<T>& PerThreadRef<T>::operator = ( RefPtr< T > data )
{
    data.ref(); // ref new

    RefPtr< T > object = get();
    tls_.set( static_cast<const void*>( data.get( )));

    object.unref(); // unref old
    return *this;
}

template< typename T >
PerThreadRef<T>& PerThreadRef<T>::operator = ( const PerThreadRef<T>& rhs )
{
    RefPtr< T > newObject = rhs.get();
    newObject.ref(); // ref new

    RefPtr< T > object = get();
    tls_.set( rhs.tls_.get( ));

    object.unref(); // unref old
    return *this;
}

template< typename T > RefPtr< const T > PerThreadRef<T>::get() const
{
    return static_cast< const T* >( tls_.get( ));
}

template< typename T > RefPtr< T > PerThreadRef<T>::get()
{
    return static_cast< T* >( tls_.get( ));
}

template< typename T >
T* PerThreadRef<T>::getPointer()
{
    return static_cast< T* >( tls_.get( ));
}

template< typename T >
T* PerThreadRef<T>::operator->()
{
    LBASSERT( tls_.get( ));
    return static_cast< T* >( tls_.get( ));
}

template< typename T >
const T* PerThreadRef<T>::operator->() const
{
    LBASSERT( tls_.get( ));
    return static_cast< const T* >( tls_.get( ));
}

template< typename T >
bool PerThreadRef<T>::operator ! () const
{
    return tls_.get() == 0;
}

template< typename T >
bool PerThreadRef<T>::isValid() const
{
    return tls_.get() != 0;
}

}
#endif //LUNCHBOX_PERTHREADREF_H
