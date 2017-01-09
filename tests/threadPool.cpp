
/* Copyright (c) 2016-2017, Mohamed-Ghaith Kaabi <mohamedghaith.kaabi@gmail.com>
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
#define BOOST_TEST_MODULE ThreadPool
#include <boost/test/unit_test.hpp>

#include <lunchbox/threadPool.h>

BOOST_AUTO_TEST_CASE( size )
{
    lunchbox::ThreadPool threadPool{3};
    BOOST_CHECK( threadPool.getSize() == 3 );
}

BOOST_AUTO_TEST_CASE( queue )
{
    lunchbox::ThreadPool threadPool{1};
    for( int i = 0; i < 10; ++i )
    {
        threadPool.postDetached( [] {
            std::this_thread::sleep_for( std::chrono::milliseconds( 50 + rand() % 50 ));
        } );
    }

    // append a dummy task
    threadPool.post( [](){}).get();
    BOOST_CHECK( !threadPool.hasPendingJobs() );
}

BOOST_AUTO_TEST_CASE( dispatcher )
{
    std::vector< std::future< int > > futures;
    lunchbox::ThreadPool threadPool{4};

    for( int i = 0; i < 10; ++i )
    {
        futures.push_back( threadPool.post( [] {
            std::this_thread::sleep_for( std::chrono::milliseconds( 50 + rand() % 50 ));
            return 42;
        }));
    }

    BOOST_CHECK( threadPool.hasPendingJobs() );

    for( auto& future : futures )
    {
        BOOST_CHECK( future.get() == 42 );
    }
}

int task()
{
    std::this_thread::sleep_for( std::chrono::milliseconds( 50 + rand() % 50 ));
    return 42;
}

BOOST_AUTO_TEST_CASE( join )
{
    std::vector< std::future< int > > futures;

    {
        lunchbox::ThreadPool threadPool{4};
        for( int i = 0; i < 100; ++i )
        {
            futures.push_back( threadPool.post( task ));
        }
    } // blocks until all tasks are done

    for( const std::future< int >& future : futures )
    {
        BOOST_CHECK( future.wait_for( std::chrono::milliseconds( 0 )) ==
                     std::future_status::ready );
    }
}
