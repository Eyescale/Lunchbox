
/* Copyright (c) 2013-2014, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_REQUEST_H
#define LUNCHBOX_REQUEST_H

#include <lunchbox/future.h>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace lunchbox
{

/**
 * A Future implementation for a RequestHandler request.
 * @version 1.9.1
 */
template< class T > class Request : public Future< T >
{
    class Impl;

public:
    /** Construct a new request. */
    Request( RequestHandler& handler, const uint32_t request );

    /**
     * Destruct and wait for completion of the request, unless relinquished.
     * @version 1.9.1
     */
    virtual ~Request();

    /** @return the identifier of the request. @version 1.9.1 */
    uint32_t getID() const;

    /**
     * Abandon the request.
     *
     * If called, wait will not be called at destruction and the request will be
     * unregistered. If the future has already been resolved this function has
     * no effect.
     * @version 1.9.1
     */
    void unregister();
};

}

// Implementation: Here be dragons

#include <lunchbox/requestHandler.h>
namespace lunchbox
{
template< class T > class Request< T >::Impl : public FutureImpl< T >
{
    typedef typename
    boost::mpl::if_< boost::is_same< T, void >, void*, T >::type value_t;

public:
    Impl( RequestHandler& handler, const uint32_t req )
        : request( req )
        , result( 0 )
        , handler_( handler )
        , done_( false )
    {}
    virtual ~Impl() {}

    const uint32_t request;
    value_t result;

    void unregister()
    {
        if( done_ )
            return;
        done_ = true;
        handler_.unregisterRequest( request );
    }

protected:
    T wait( const uint32_t timeout ) final
    {
        if( !done_ )
        {
            if ( !handler_.waitRequest( request, result, timeout ))
                throw FutureTimeout();
            done_ = true;
        }
        return result;
    }

    bool isReady() const final
    {
        return done_ || handler_.isRequestReady( request );
    }

private:
    RequestHandler& handler_;
    bool done_; //!< waitRequest finished
};

template<> inline void Request< void >::Impl::wait( const uint32_t timeout )
{
    if( !done_ )
    {
        if ( !handler_.waitRequest( request, result, timeout ))
            throw FutureTimeout();
        done_ = true;
    }
}

template< class T > inline
Request< T >::Request( RequestHandler& handler, const uint32_t request )
    : Future< T >( new Impl( handler, request ))
{}

template< class T > inline Request< T >::~Request()
{
    this->wait();
}

template< class T > inline uint32_t Request< T >::getID() const
{
    return static_cast< const Impl* >( this->impl_.get( ))->request;
}

template< class T > inline void Request< T >::unregister()
{
    static_cast< Impl* >( this->impl_.get( ))->unregister();
}

}

#endif //LUNCHBOX_REQUEST_H
