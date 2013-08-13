
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_REQUESTFUTURE_H
#define LUNCHBOX_REQUESTFUTURE_H

#include <lunchbox/futureFunction.h>
#include <boost/bind.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace lunchbox
{

/**
 * A Future implementation for a RequestHandler request.
 * @version 1.9.1
 */
template< class T > class RequestFuture : public Future< bool >
{
    class Impl : public FutureFunction< bool >
    {
        typedef typename boost::mpl::if_< boost::is_same< T, void >,
                                          void*, T >::type value_t;
    public:
        Impl( RequestHandler& handler, const uint32_t req )
            : FutureFunction< bool >(
                          boost::bind( &RequestFuture< T >::Impl::wait_, this ))
            , request( req )
            , result( 0 )
            , handler_( handler )
        {}

        const uint32_t request;
        value_t result;

    private:
        RequestHandler& handler_;

        bool wait_();
    };

public:
    RequestFuture( RequestHandler& handler, const uint32_t request )
        : Future< bool >( new Impl( handler, request ))
    {}

    virtual ~RequestFuture() { wait(); }

    uint32_t getID() const
        { return static_cast< const Impl* >( impl_.get( ))->request; }

    T get()
    {
        LBCHECK( wait( ));
        return static_cast< Impl* >( impl_.get( ))->result;
    }
};

}

#include <lunchbox/requestHandler.h>
namespace lunchbox
{
template< class T > inline bool RequestFuture< T >::Impl::wait_()
{
    return handler_.waitRequest( request, result );
}
}

#endif //LUNCHBOX_REQUESTFUTURE_H
