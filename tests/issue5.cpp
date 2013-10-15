
/* Copyright (c) 2012, Stefan.Eilemann@epfl.ch
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

#include <test.h>

#include <lunchbox/clock.h>
#include <lunchbox/condition.h>
#include <lunchbox/rng.h>

#include <iostream>

int main( int, char** )
{
    lunchbox::Condition condition;
    lunchbox::Clock clock;
    {
        TEST( !condition.timedWait( 2345 ));
        const float time = clock.getTimef();
        TESTINFO( time > 2344.f, time );
    }
    lunchbox::RNG rng;
    unsigned nTests = 30;
    while( nTests-- )
    {
        const uint32_t timeout = rng.get<uint8_t>() + 2;

        clock.reset();
        TEST( !condition.timedWait( timeout ));
        const float time = clock.getTimef();

        TESTINFO( time > float( timeout - 1 ), time << " < " << timeout );
    }

    return EXIT_SUCCESS;
}
