
/* Copyright (c) 2013, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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
#include <lunchbox/unorderedIntervalSet.h>

int main( int, char** )
{
    typedef lunchbox::UnorderedIntervalSet< size_t > SetType;
    SetType set;
    TEST( set.empty( ));

    set.insert( 2 );
    TEST( set.size() == 1 );

    set.insert( 1, 5 );
    TEST( set.exists( 3 ));
    TEST( set.size() == 5 );

    set.insert( 1, 5 );
    TEST( set.size() == 5 );

    set.insert( 2, 4 );
    TEST( set.size() == 5 );

    TEST( set.find( 0 ) == set.end( ));
    TEST( *set.find( 3 ) == 3 );
    TEST( set.find( 6 ) == set.end( ));

    size_t i = 1;
    for( SetType::const_iterator it = set.begin(); it != set.end(); ++it, ++i )
        TEST( *it == i );

    set.erase( 3, 4 );
    TEST( set.size() == 3 );

    set.erase( 2 );
    TEST( set.size() == 2 );

    TEST( set.exists( 1 ));
    TEST( set.exists( 5 ));

    set.clear();
    TEST( set.empty( ));

    set.insert( 0, 1 );
    TEST( set.size() == 2 );
    set.insert( 3, 5 );
    TEST( set.size() == 5 );
    SetType::const_iterator it = set.begin();
    TEST( *it == 0 );
    ++it;
    TEST( *it == 1 );
    ++it;
    TEST( *it == 3 );
    ++it;
    TEST( *it == 4 );
    ++it;
    TEST( *it == 5 );

    set.erase( 4, 7 );
    TEST( set.size() == 3 );

    set.erase( 40, 70 );
    TEST( set.size() == 3 );

    return EXIT_SUCCESS;
}
