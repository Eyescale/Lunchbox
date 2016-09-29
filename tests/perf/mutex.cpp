
/* Copyright (c) 2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#define TEST_RUNTIME 600 // seconds, needed for NighlyMemoryCheck
#include <lunchbox/test.h>

#include <lunchbox/clock.h>
#include <lunchbox/debug.h>
#include <lunchbox/init.h>

#include <mutex>
#include <iostream>

#define MAXTHREADS 256
#define TIME       500  // ms

lunchbox::Clock _clock;
bool _running = false;

template< class T > class Thread : public lunchbox::Thread
{
public:
    Thread() : mutex( 0 ), ops( 0 ) {}

    T* mutex;
    size_t ops;

    virtual void run()
    {
        ops = 0;
        while( LB_LIKELY( _running ))
        {
            mutex->lock();
            mutex->unlock();
            ++ops;
        }
    }
};

template< class T > void _test()
{
    const size_t nThreads = 16;

    T mutex;
    mutex.lock();

    Thread< T > threads[MAXTHREADS];
    for( size_t i = 1; i <= nThreads; i = i << 1 )
    {
        _running = true;
        for( size_t j = 0; j < i; ++j )
        {
            threads[j].mutex = &mutex;
            TEST( threads[j].start( ));
        }
        lunchbox::sleep( 10 ); // let threads initialize

        _clock.reset();
        mutex.unlock();
        lunchbox::sleep( TIME ); // let threads run
        _running = false;

        for( size_t j = 0; j < i; ++j )
            TEST( threads[j].join( ));
        const float time = _clock.getTimef();

        mutex.lock();

        size_t ops = 0;
        for( size_t j = 0; j < nThreads; ++j )
            ops += threads[j].ops;

        std::cout << std::setw(20) << lunchbox::className( mutex ) << ", "
                  << std::setw(12) << /*set, test, unset*/ 3 * ops / time
                  << ", " << std::setw(3) << i << std::endl;
    }
}

int main( int argc, char **argv )
{
    TEST( lunchbox::init( argc, argv ));

    std::cout << "               Class,       ops/ms, threads" << std::endl;
    _test< std::mutex >();
    std::cout << std::endl;

    _test< std::timed_mutex >();
    TEST( lunchbox::exit( ));
    return EXIT_SUCCESS;
}
