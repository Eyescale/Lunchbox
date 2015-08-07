
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
#include <iostream>

using servus::uint128_t;

lunchbox::Monitor< int64_t > monitor;
lunchbox::Monitor< bool > boolMonitor;

class Thread1 : public lunchbox::Thread
{
public:
    explicit Thread1( int64_t loops ) : _loops( loops ) {}
    virtual ~Thread1() {}
    virtual void run()
        {
            int64_t nOps = _loops;
            lunchbox::Clock clock;
            while( nOps-- )
            {
                monitor.waitEQ( nOps );
                monitor = -nOps;
            }

            const float time = clock.getTimef();
            std::cout << 2*_loops / time << " ops/ms" << std::endl;
        }
private:
    int64_t _loops;
};

void testSimpleMonitor()
{
    TEST( !boolMonitor );
    boolMonitor = true;
    TEST( boolMonitor );

    const int64_t loops = 200000;
    int64_t nOps = loops;
    Thread1 waiter( nOps );

    TEST( waiter.start( ));
    lunchbox::Clock clock;

    while( nOps-- )
    {
        monitor = nOps;
        monitor.waitEQ( -nOps );
    }

    const float time = clock.getTimef();

    TEST( waiter.join( ));
    std::cout << 2*loops / time << " ops/ms" << std::endl;
}

class Thread2 : public lunchbox::Thread
{
public:
    explicit Thread2( size_t loops )
        : innerLoops( 0 )
        , _outerLoops( loops )
    {}

    virtual ~Thread2() {}
    virtual void run()
        {
            lunchbox::Clock clock;
            size_t ops = 0;
            for( size_t k = 0; k != 2; ++k )
            {
                for( size_t i = 0; i != _outerLoops; ++i )
                {
                    const int64_t loops = innerLoops.waitNE( 0 );
                    innerLoops = 0;
                    for( int64_t j = 0; j != loops; ++j )
                    {
                        if (innerLoops > 0)
                            abort();
                        ++monitor;
                    }
                    ops += loops * 2 + 2;
                }
            }

            for( size_t k = 0; k != 2; ++k )
            {
                for( size_t i = 0; i != _outerLoops; ++i )
                {
                    const int64_t loops = innerLoops.waitNE( 0 );
                    innerLoops = 0;
                    for( int64_t j = 0; j != loops; ++j )
                    {
                        if (innerLoops > 0)
                            abort();
                        --monitor;
                    }
                    ops += loops * 2 + 2;
                }
            }

            const float time = clock.getTimef();
            std::cout << ops/time << " ops/ms" << std::endl;
        }

    lunchbox::Monitor< int64_t > innerLoops;
private:
    size_t _outerLoops;
};

void testMonitorComparisons()
{
    boolMonitor = true;
    const int64_t loops = 1000;
    Thread2 waiter( loops );
    TEST( waiter.start( ));

    for( int64_t i = 0; i != loops; ++i )
    {
        monitor = 0;
        waiter.innerLoops = i + 1;
        TEST( monitor.waitGE( i + 1 ) >= i + 1 );
        TEST( monitor >= i + 1 );
    }

    for( int64_t i = 0; i != loops; ++i )
    {
        monitor = 0;
        waiter.innerLoops = i + 1;
        TEST( monitor.waitGT( i ) > i );
        TEST( monitor > i );
    }

    for( int64_t i = 0; i != loops; ++i )
    {
        monitor = i + 1;
        waiter.innerLoops = i + 1;
        TEST( monitor.waitLE( 0 ) <= 0 );
        TEST( monitor <= 0 );
    }

    for( int64_t i = 0; i != loops; ++i )
    {
        monitor = i + 1;
        waiter.innerLoops = i + 1;
        TEST( monitor.waitLT( 1 ) < 1 );
        TEST( monitor < 1 );
    }

    TEST( waiter.join( ));
}

void testTimedMonitorComparisons()
{
    boolMonitor = true;
    const size_t loops = 1000;
    Thread2 waiter( loops );
    TEST( waiter.start( ));
    const uint32_t timeout = 1;

    for( size_t i = 0; i != loops; ++i )
    {
        monitor = 0;
        waiter.innerLoops = i + 1;
        while( !monitor.timedWaitGE( i + 1, timeout ))
            ;
        TEST( monitor >= i + 1 );
    }
    for( size_t i = 0; i != loops; ++i )
    {
        monitor = 0;
        waiter.innerLoops = i + 1;
        while( !monitor.timedWaitGT( i, timeout ))
            ;
        TEST( monitor > i );
    }
    for( size_t i = 0; i != loops; ++i )
    {
        monitor = i + 1;
        waiter.innerLoops = i + 1;
        while( !monitor.timedWaitLE( 0, timeout ))
            ;
        TEST( monitor <= 0 );
    }
    for( size_t i = 0; i != loops; ++i )
    {
        monitor = i + 1;
        waiter.innerLoops = i + 1;
        while( !monitor.timedWaitLT( 1, timeout ))
            ;
        TEST( monitor < 1 );
    }

    TEST( waiter.join( ));
}

int main( int, char** )
{
    testSimpleMonitor();
    testMonitorComparisons();
    testTimedMonitorComparisons();
    return EXIT_SUCCESS;
}
