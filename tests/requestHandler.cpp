
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

#ifdef WARN_DEPRECATED
#  undef WARN_DEPRECATED
#endif

#include "test.h"

#include <lunchbox/mtQueue.h>
#include <lunchbox/requestHandler.h>
#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <servus/uint128_t.h>

using servus::uint128_t;

lunchbox::RequestHandler handler_;
lunchbox::MTQueue< uint32_t > requestQ_;
const uint128_t uuid = servus::make_UUID();

class Thread : public lunchbox::Thread
{
public:
    virtual void run() final
    {
        const bool boolIn = true;
        uint8_t* payload = (uint8_t*)42;

        uint32_t request = requestQ_.pop();
        TEST( handler_.getRequestData( request ) == ++payload );
        handler_.serveRequest( request, boolIn );

        const uint32_t uint32In = 0xC0FFEE;
        request = requestQ_.pop();
        TEST( handler_.getRequestData( request ) == ++payload );
        handler_.serveRequest( request, uint32In );

        request = requestQ_.pop();
        TEST( handler_.getRequestData( request ) == ++payload );
        handler_.serveRequest( request );

        request = requestQ_.pop();
        TESTINFO( handler_.getRequestData( request ) == ++payload,
                  (size_t)handler_.getRequestData( request ) << " for " <<
                  request );
        handler_.serveRequest( request );

        request = requestQ_.pop();
        TEST( handler_.getRequestData( request ) == ++payload );
        handler_.serveRequest( request, uuid );
    }
};

int main( int, char** )
{
    uint8_t* payload = (uint8_t*)42;
    Thread thread;
    thread.start();

    uint32_t request = handler_.registerRequest( ++payload );
    requestQ_.push( request );
    bool boolOut = false;
    TEST( handler_.waitRequest( request, boolOut ));
    TEST( boolOut == true );

    lunchbox::Request< uint32_t > future =
        handler_.registerRequest< uint32_t >( ++payload );
    requestQ_.push( future.getID( ));

    request = handler_.registerRequest( ++payload );
    requestQ_.push( request );
    TEST( handler_.waitRequest( request ));

    lunchbox::Request< void > voidFuture =
        handler_.registerRequest< void >( ++payload );
    lunchbox::Request< uint128_t > uint128Future =
        handler_.registerRequest< uint128_t >( ++payload );

    requestQ_.push( voidFuture.getID( ));
    requestQ_.push( uint128Future.getID( ));

    TEST( future.wait( ) == 0xC0FFEE );
    TEST( future.wait( ));
    TEST( uint128Future.wait() == uuid );
    TEST( handler_.isRequestServed( voidFuture.getID( )));

    TEST( thread.join( ));
    return EXIT_SUCCESS;
}
