
/* Copyright (c) 2012-2014, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <servus/uint128_t.h>
#include <iostream>

#define NLOOPS 200000

using servus::uint128_t;

lunchbox::Monitor< uint128_t > monitor;
const uint128_t big( 10, 10 );

class Thread : public lunchbox::Thread
{
public:
    virtual ~Thread() {}
    virtual void run()
        {
            const uint128_t invalid1( 0, 0 );
            const uint128_t invalid2( 1, 1 );

            while( monitor != big )
            {
                const uint128_t& result = monitor.waitLE( big );
                TEST( result != invalid1 );
                TEST( result != invalid2 );
                TEST( monitor != invalid1 );
                TEST( monitor != invalid2 );
            }
        }
};

int main( int, char** )
{
    int64_t nOps = NLOOPS;
    const uint128_t valid1( 1, 0 );
    const uint128_t valid2( 0, 1 );
    Thread thread;

    monitor = valid1;

    TEST( thread.start( ));
    while( --nOps )
    {
        monitor = valid1; // cppcheck-suppress redundantAssignment
        monitor = valid2;
    }
    monitor = big;
    TEST( thread.join( ));

    return EXIT_SUCCESS;
}
