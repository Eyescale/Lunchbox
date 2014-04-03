
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
#include <boost/bind.hpp>
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
    class Impl : public FutureImpl< T >
    {
        typedef typename boost::mpl::if_< boost::is_same< T, void >,
                                          void*, T >::type value_t;
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

    protected:
        T wait( const uint32_t timeout ) final;
        bool isReady() const final;

    private:
        RequestHandler& handler_;
        bool done_; //!< waitRequest finished
    };

public:
    Request( RequestHandler& handler, const uint32_t request )
        : Future< T >( new Impl( handler, request ))
    {}

    virtual ~Request() { wait(); }

    uint32_t getID() const
        { return static_cast< const Impl* >( this->impl_.get( ))->request; }

    T get( boost::enable_if< boost::is_same< T, void > >* = 0)
    {
        return this->wait();
    }
};

}

#include <lunchbox/requestHandler.h>
namespace lunchbox
{
template< class T > inline T Request< T >::Impl::wait(
    const uint32_t timeout )
{
    if( !done_ )
    {
        if ( !handler_.waitRequest( request, result, timeout ))
            throw typename Future< T >::timeout();
        done_ = true;
    }
    return result;
}

template<> inline void Request< void >::Impl::wait(
    const uint32_t timeout )
{
    if( !done_ )
    {
        if ( !handler_.waitRequest( request, result, timeout ))
            throw typename Future< void >::timeout();
        done_ = true;
    }
}

template< class T > inline bool Request< T >::Impl::isReady() const
{
    return done_ || handler_.isRequestReady( request );
}

}

#endif //LUNCHBOX_REQUEST_H
