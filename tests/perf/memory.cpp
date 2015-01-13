
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

#include <lunchbox/clock.h>
#include <lunchbox/debug.h>
#include <lunchbox/init.h>

int main( int argc, char **argv )
{
    TEST( lunchbox::init( argc, argv ));

    std::cout << "Operation, GB/s" << std::endl;
    void* const from = ::malloc( LB_1GB );
    void* const to = ::malloc( LB_1GB );
    // touch all pages once
    ::memset( from, 0, LB_1GB );
    ::memset( to, 0, LB_1GB );

    lunchbox::Clock clock;

    ::memcpy( to, from, LB_1GB );
    std::cout << "memcpy,    " << 1.f / (clock.resetTimef() / 1000.f)
              << std::endl;
    ::memmove( to, from, LB_1GB );
    std::cout << "memmove,   " << 1.f / (clock.resetTimef() / 1000.f)
              << std::endl;
    ::memset( to, 42, LB_1GB );
    std::cout << "memset,    " << 1.f / (clock.resetTimef() / 1000.f)
              << std::endl;

    free( to );
    free( from );
    TEST( lunchbox::exit( ));
    return EXIT_SUCCESS;
}
