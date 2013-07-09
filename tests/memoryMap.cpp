
/* Copyright (c) 2013 Stefan.Eilemann@epfl.ch
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

// Tests the functionality of MemoryMap

#include <test.h>
#include <lunchbox/memoryMap.h>

using lunchbox::MemoryMap;
#define MAP_SIZE LB_10MB
#define STRIDE 23721

int main( int argc, char **argv )
{
    MemoryMap map( "foo.mmap", MAP_SIZE );
    TESTINFO( map.getSize() == MAP_SIZE, map.getSize( ));

    uint8_t* writePtr = map.getAddress< uint8_t >();
    TEST( writePtr );

    for( size_t i=0; i < MAP_SIZE; i += STRIDE )
        writePtr[i] = uint8_t( i );
    map.unmap();

    const void* noPtr = map.map( "foo.map" );
    TEST( !noPtr );
    TEST( map.getSize() == 0 );

    map.map( "foo.mmap" );
    const uint8_t* readPtr = map.getAddress< uint8_t >();
    TEST( readPtr );
    TEST( map.getSize() == MAP_SIZE );

    for( size_t i=0; i < MAP_SIZE; i += STRIDE )
        TEST( readPtr[i] == uint8_t( i ));

    return EXIT_SUCCESS;
}
