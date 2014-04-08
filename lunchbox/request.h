
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
            , relinquished_( false )
        {}
        virtual ~Impl() {}

        const uint32_t request;
        value_t result;

        void relinquish()
        {
            relinquished_ = true;
        }

        bool isRelinquished() const
        {
            return relinquished_;
        }

    protected:
        T wait( const uint32_t timeout ) final;
        bool isReady() const final;

    private:
        RequestHandler& handler_;
        bool done_; //!< waitRequest finished
        bool relinquished_;
    };

public:
    /** Exception throw by operations that invoke wait once the request has
        been relinquished */
    class relinquished : public std::runtime_error
    {
    public:
        relinquished() : std::runtime_error("") {}
    };

    Request( RequestHandler& handler, const uint32_t request )
        : Future< T >( new Impl( handler, request ))
    {}

    virtual ~Request()
    {
        if( !static_cast< const Impl* >( this->impl_.get( ))->isRelinquished( ))
            wait();
    }

    uint32_t getID() const
        { return static_cast< const Impl* >( this->impl_.get( ))->request; }

    T get()
    {
        return this->wait();
    }

    /** If called, wait will not be called at destruction and get will throw
        an the relinquished exception. If the future has already been
        resolved this function has no effect. */
    void relinquish()
    {
        static_cast< Impl* >( this->impl_.get( ))->relinquish();
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
        if( relinquished_ )
            throw relinquished();

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
        if( relinquished_ )
            throw relinquished();

        if ( !handler_.waitRequest( request, result, timeout ))
            throw typename Future< void >::timeout();
        done_ = true;
    }
}

template< class T > inline bool Request< T >::Impl::isReady() const
{
    return done_ || ( !relinquished_ && handler_.isRequestReady( request ));
}

}

#endif //LUNCHBOX_REQUEST_H
