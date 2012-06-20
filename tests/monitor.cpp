
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#define TEST_RUNTIME 300 // seconds
#include "test.h"

#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <lunchbox/thread.h>
#include <lunchbox/uint128_t.h>
#include <iostream>

#define NLOOPS 200000

using lunchbox::uint128_t;

lunchbox::Monitor< int64_t > monitor;
lunchbox::Monitor< uint128_t > issue1;
const uint128_t big( 10, 10 );

class Thread : public lunchbox::Thread
{
public:
    virtual ~Thread() {}
    virtual void run()
        {
            int64_t nOps = NLOOPS;

            lunchbox::Clock clock;
            while( nOps-- )
            {
                monitor.waitEQ( nOps );
                monitor = -nOps;
            }

            const float time = clock.getTimef();
            std::cout << 2*NLOOPS/time << " ops/ms" << std::endl;
        }
};

class Issue1 : public lunchbox::Thread
{
public:
    virtual ~Issue1() {}
    virtual void run()
        {
            const uint128_t invalid1( 0, 0 );
            const uint128_t invalid2( 1, 1 );

            while( issue1 != big )
            {
                const uint128_t& result = issue1.waitLE( big );
                TEST( result != invalid1 );
                TEST( result != invalid2 );
                TEST( issue1 != invalid1 );
                TEST( issue1 != invalid2 );
            }
        }
};

int main( int argc, char **argv )
{
    Thread waiter;
    int64_t nOps = NLOOPS;

    TEST( waiter.start( ));
    lunchbox::Clock clock;

    while( nOps-- )
    {
        monitor = nOps;
        monitor.waitEQ( -nOps );
    }

    const float time = clock.getTimef();

    TEST( waiter.join( ));
    std::cout << 2*NLOOPS/time << " ops/ms" << std::endl;

    const uint128_t valid1( 1, 0 );
    const uint128_t valid2( 0, 1 );

    issue1 = valid1;
    nOps = NLOOPS;
    Issue1 thread;

    TEST( thread.start( ));
    while( --nOps )
    {
        issue1 = valid1;
        issue1 = valid2;
    }
    issue1 = big;
    TEST( thread.join( ));

    return EXIT_SUCCESS;
}
