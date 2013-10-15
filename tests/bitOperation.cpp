
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
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
#include <lunchbox/bitOperation.h>

int main( int, char** )
{
    TESTINFO( lunchbox::getIndexOfLastBit( 0u ) == -1,
              lunchbox::getIndexOfLastBit( 0u ));
    TESTINFO( lunchbox::getIndexOfLastBit( 42u ) == 5,
              lunchbox::getIndexOfLastBit( 42u ));
    TESTINFO( lunchbox::getIndexOfLastBit( LB_BIT12 ) == 11,
              lunchbox::getIndexOfLastBit( LB_BIT12 ));
    TESTINFO( lunchbox::getIndexOfLastBit( LB_BIT48 ) == 47,
              lunchbox::getIndexOfLastBit( LB_BIT48 ));

    uint16_t twoByte = 1;
    lunchbox::byteswap( twoByte );
    TESTINFO( twoByte == LB_BIT9, twoByte );

    uint32_t fourByte = 1;
    lunchbox::byteswap( fourByte );
    TESTINFO( fourByte == LB_BIT25, fourByte );

    uint64_t eightByte = 1;
    lunchbox::byteswap( eightByte );
    TESTINFO( eightByte == LB_BIT57, eightByte );

    return EXIT_SUCCESS;
}
