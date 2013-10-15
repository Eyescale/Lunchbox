
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

#ifndef LUNCHBOX_PERTHREAD_H
#define LUNCHBOX_PERTHREAD_H

#include <lunchbox/tls.h> // member

namespace lunchbox
{
namespace detail { class PerThread; }

/** Default PerThread destructor deleting the object. @version 1.1.2 */
template< class T > void perThreadDelete( T* object ) { delete object; }

/** Empty PerThread destructor. @version 1.1.2 */
template< class T > void perThreadNoDelete( T* ) {}

/**
 * Implements thread-specific storage for C++ objects.
 *
 * The default destructor function deletes the object on thread exit.
 *
 * @param T the type of data to store in thread-local storage
 * @param D the destructor callback function.
 */
template< class T, void (*D)( T* ) = &perThreadDelete< T > >
class PerThread : public NonCopyable
{
public:
    /** Construct a new per-thread variable. @version 1.0 */
    PerThread();
    /** Destruct the per-thread variable. @version 1.0 */
    ~PerThread();

    /** Assign an object to the thread-local storage. @version 1.0 */
    PerThread<T, D>& operator = ( const T* data );
    /** Assign an object from another thread-local storage. @version 1.0 */
    PerThread<T, D>& operator = ( const PerThread<T, D>& rhs );

    /** @return the held object pointer. @version 1.0 */
    T* get();
    /** @return the held object pointer. @version 1.0 */
    const T* get() const;
    /** Access the thread-local object. @version 1.0 */
    T* operator->();
    /** Access the thread-local object. @version 1.0 */
    const T* operator->() const;

    /** @return the held object reference. @version 1.0 */
    T& operator*()
        { LBASSERTINFO( get(), className( this )); return *get(); }
    /** @return the held object reference. @version 1.0 */
    const T& operator*() const
        { LBASSERTINFO( get(), className( this )); return *get(); }

    /**
     * @return true if the thread-local variables hold the same object.
     * @version 1.0
     */
    bool operator == ( const PerThread& rhs ) const
        { return ( get() == rhs.get( )); }

    /**
     * @return true if the thread-local variable holds the same object.
     * @version 1.0
     */
    bool operator == ( const T* rhs ) const { return ( get()==rhs ); }

    /**
     * @return true if the thread-local variable holds another object.
     * @version 1.0
     */
    bool operator != ( const T* rhs ) const { return ( get()!=rhs ); }

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
#endif //LUNCHBOX_PERTHREAD_H
