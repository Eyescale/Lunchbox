
/* Copyright (c) 2010-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <pthread.h>
#include <test.h>
#include <lunchbox/clock.h>
#include <lunchbox/compiler.h>
#include <lunchbox/mtQueue.h>
#include <lunchbox/thread.h>
#include <iostream>

#define NOPS 100000
#define NTHREADS 4

lunchbox::MTQueue< uint64_t > queue;
lunchbox::MTQueue< uint64_t >::Group group( NTHREADS + 1 );

#ifdef LB_GCC_4_6_OR_LATER
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
class ReadThread : public lunchbox::Thread
{
public:
    virtual ~ReadThread() {}
    virtual void run() { run_(); }

    static void run_()
    {
        uint64_t item = 0xffffffffffffffffull;
#ifndef NDEBUG
        uint64_t last = 0;
#endif
        while( queue.popBarrier( item, group ))
        {
#ifndef NDEBUG
            TESTINFO( last < item, last << " >= " << item );
            last = item;
#endif
        }
        TEST( queue.isEmpty( ));
    }
};

int main( int argc, char **argv )
{
    ReadThread reader[ NTHREADS ];
    for( size_t i = 0; i < NTHREADS; ++i )
        TEST( reader[i].start( ));

    lunchbox::Clock clock;
    for( size_t i = 1 ; i < NOPS; ++i )
        queue.push( i );
    const float time = clock.getTimef();

    ReadThread::run_();

    for( size_t i = 0; i < NTHREADS; ++i )
        TEST( reader[i].join( ));

    std::cout << NOPS/time << " writes/ms" << std::endl;
    return EXIT_SUCCESS;
}

