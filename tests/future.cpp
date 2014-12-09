
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
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

#define BOOST_TEST_MODULE Future

#include <lunchbox/clock.h>
#include <lunchbox/readyFuture.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( readyFutures )
{
    lunchbox::f_bool_t ok = lunchbox::makeTrueFuture();
    BOOST_CHECK( ok == true );
    BOOST_CHECK( ok.isReady( ));

    lunchbox::f_bool_t nok = lunchbox::makeFalseFuture();
    BOOST_CHECK( nok == false );
    BOOST_CHECK( nok.isReady( ));
}

static lunchbox::f_bool_t futureFunction()
{
    return lunchbox::makeTrueFuture();;
}

static bool presentFunction()
{
    return true;
}

BOOST_AUTO_TEST_CASE( perfFuture )
{
    static const size_t nLoops = 1000000;

    lunchbox::Clock clock;
    for( size_t i = 0; i < nLoops; ++i )
        futureFunction();
    const float futureSync = clock.resetTimef();

    std::vector< lunchbox::f_bool_t > futures;
    futures.reserve( nLoops );
    for( size_t i = 0; i < nLoops; ++i )
        futures.push_back( futureFunction( ));
    futures.clear();
    const float futureASync = clock.resetTimef();

    for( size_t i = 0; i < nLoops; ++i )
        presentFunction();
    const float present = clock.resetTimef();

    std::cout << nLoops / futureSync / 1000.f << " sync futures, "
              << nLoops / futureASync / 1000.f << " async futures, "
              << nLoops / present / 1000.f << " normal calls/us" << std::endl;
}
