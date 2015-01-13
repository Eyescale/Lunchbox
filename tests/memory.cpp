
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

    std::cout << "Operation, MB/s" << std::endl;
    void* const from = ::malloc( LB_100MB );
    void* const to = ::malloc( LB_100MB );
    // touch all pages once
    ::memset( from, 0, LB_100MB );
    ::memset( to, 0, LB_100MB );

    lunchbox::Clock clock;

    ::memcpy( to, from, LB_100MB );
    std::cout << "memcpy,    " << 100 /*MB*/ / (clock.resetTimef() / 1000.f)
              << std::endl;
    ::memmove( to, from, LB_100MB );
    std::cout << "memmove,   " << 100 /*MB*/ / (clock.resetTimef() / 1000.f)
              << std::endl;
    ::memset( to, 42, LB_100MB );
    std::cout << "memset,    " << 100 /*MB*/ / (clock.resetTimef() / 1000.f)
              << std::endl;

    free( to );
    free( from );
    TEST( lunchbox::exit( ));
    return EXIT_SUCCESS;
}
