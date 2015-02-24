
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
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

#include "test.h"

#include <lunchbox/lunchbox.h>
#include <string.h>

enum Task
{
    TASK_INITIAL,
    TASK_MEMCPY,
    TASK_MEMMOVE,
    TASK_MEMSET
};
lunchbox::Monitor< Task > task;
static const size_t nThreads = 16;

class TaskThread : public lunchbox::Thread
{
public:
    TaskThread() : size( LB_1GB ) {}

    void run() override
    {
        // alloc and touch all pages once
        void* const from = ::malloc( size );
        void* const to = ::malloc( size );
        ::memset( from, 0, size );
        ::memset( to, 0, size );

        lunchbox::Clock clock;

        task.waitGE( TASK_MEMCPY );
        clock.reset();
        ::memcpy( to, from, size );
        memcpyTime = clock.getTimef();

        task.waitGE( TASK_MEMMOVE );
        clock.reset();
        ::memmove( to, from, size );
        memmoveTime = clock.getTimef();

        task.waitGE( TASK_MEMSET );
        clock.reset();
        ::memset( to, 42, size );
        memsetTime = clock.getTimef();

        free( to );
        free( from );
    }

    size_t size;
    float memcpyTime;
    float memmoveTime;
    float memsetTime;
};

int main( int argc, char **argv )
{
    TEST( lunchbox::init( argc, argv ));

    TaskThread thread;
    task = TASK_MEMSET;
    thread.run();
    std::cout << "#threads, memcpy, memmove, memset (GB/s)" << std::endl;

    TaskThread threads[ nThreads ];
    for( size_t i = 1; i <= nThreads; i = i << 1 )
    {
        task = TASK_INITIAL;
        for( size_t j = 0; j < i; ++j )
        {
            threads[ j ].size = LB_1GB / i;
            threads[ j ].start();
        }

        lunchbox::sleep( 500 );
        task = TASK_MEMCPY;
        lunchbox::sleep( 500 );
        task = TASK_MEMMOVE;
        lunchbox::sleep( 500 );
        task = TASK_MEMSET;

        float memcpyTime = 0.f;
        float memmoveTime = 0.f;
        float memsetTime = 0.f;

        for( size_t j = 0; j < i; ++j )
        {
            threads[ j ].join();
            TESTINFO( threads[ j ].memcpyTime < 500.f,
                      threads[ j ].memcpyTime );
            TESTINFO( threads[ j ].memmoveTime < 500.f,
                      threads[ j ].memmoveTime);
            TESTINFO( threads[ j ].memsetTime < 500.f,
                      threads[ j ].memsetTime);

            memcpyTime = std::max( memcpyTime, threads[ j ].memcpyTime );
            memmoveTime = std::max( memmoveTime, threads[ j ].memmoveTime );
            memsetTime = std::max( memsetTime, threads[ j ].memsetTime );
        }
        std::cout << i << ", " << 1.f / memcpyTime * 1000.f << ", "
                  << 1.f / memmoveTime * 1000.f << ", "
                  << 1.f / memsetTime * 1000.f << std::endl;
    }

    TEST( lunchbox::exit( ));
    return EXIT_SUCCESS;
}
