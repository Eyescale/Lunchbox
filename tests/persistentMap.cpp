
/* Copyright (c) 2014 Stefan.Eilemann@epfl.ch
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
#include <lunchbox/persistentMap.h>
#ifdef LUNCHBOX_USE_LEVELDB
#  include <leveldb/db.h>
#endif
#include <stdexcept>

using lunchbox::PersistentMap;

const int ints[] = { 17, 53, 42 };
const size_t numInts = sizeof( ints ) / sizeof( int );

void setup( const std::string& uri )
{
    PersistentMap map( uri );
    TEST( map.insert( "foo", "bar" ));
    TEST( map[ "foo" ] == "bar" );
    TEST( map[ "bar" ].empty( ));

    std::vector< int > vector( ints, ints + numInts );
    TEST( map.insert( "std::vector< int >", vector ));

    std::set< int > set( ints, ints + numInts );
    TEST( map.insert( "std::set< int >", set ));
}

void read( const std::string& uri )
{
    PersistentMap map( uri );
    TEST( map[ "foo" ] == "bar" );
    TEST( map[ "bar" ].empty( ));

    const std::vector< int >& vector =
        map.getVector< int >( "std::vector< int >" );
    TEST( vector.size() ==  numInts );
    for( size_t i = 0; i < numInts; ++i )
        TEST( vector[ i ] == ints[i] );

    const std::set< int >& set = map.getSet< int >( "std::set< int >" );
    TEST( set.size() ==  numInts );
    for( size_t i = 0; i < numInts; ++i )
        TEST( set.find( ints[i] ) != set.end( ));
}

void testGenericFailures()
{
    try
    {
        setup( "foobar://" );
    }
    catch( const std::runtime_error& )
    {
        return;
    }
    TESTINFO( false, "Missing exception" );
}

void testLevelDBFailures()
{
#ifdef LUNCHBOX_USE_LEVELDB
    try
    {
        setup( "leveldb:///doesnotexist/deadbeef/coffee" );
    }
    catch( const leveldb::Status& status )
    {
        return;
    }
    TESTINFO( false, "Missing exception" );
#endif
}

int main( int, char** )
{
    try
    {
#ifdef LUNCHBOX_USE_LEVELDB
        setup( "" );
        setup( "leveldb://" );
        setup( "leveldb://persistentMap2.leveldb" );
        read( "" );
        read( "leveldb://" );
        read( "leveldb://persistentMap2.leveldb" );
#endif
    }
#ifdef LUNCHBOX_USE_LEVELDB
    catch( const leveldb::Status& status )
    {
        TESTINFO( !"exception", status.ToString( ));
    }
#endif
    catch( const std::runtime_error& error )
    {
        TESTINFO( !"exception", error.what( ));
    }

    testGenericFailures();
    testLevelDBFailures();

    return EXIT_SUCCESS;
}
