
/* Copyright (c) 2014-2015, Stefan.Eilemann@epfl.ch
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

#define TEST_RUNTIME 240 //seconds
#include <test.h>
#include <lunchbox/clock.h>
#include <lunchbox/os.h>
#include <lunchbox/persistentMap.h>
#ifdef LUNCHBOX_USE_LEVELDB
#  include <leveldb/db.h>
#endif
#ifdef LUNCHBOX_USE_SKV
#  include <FxLogger/FxLogger.hpp>
#endif
#include <boost/format.hpp>
#include <stdexcept>
#include <deque>

using lunchbox::PersistentMap;

const int ints[] = { 17, 53, 42, 65535, 32768 };
const size_t numInts = sizeof( ints ) / sizeof( int );
const int64_t loopTime = 1000;

template< class T > void insertVector( PersistentMap& map )
{
    std::vector< T > vector;
    for( size_t i = 0; i < numInts; ++i )
        vector.push_back( T( ints[ i ] ));

    TEST( map.insert( typeid( vector ).name(), vector ));
}

template< class T > void readVector( PersistentMap& map )
{
    const std::vector< T >& vector =
        map.getVector< T >( typeid( vector ).name( ));
    TESTINFO( vector.size() ==  numInts, vector.size() << " != " << numInts );
    for( size_t i = 0; i < numInts; ++i )
        TEST( vector[ i ] == T( ints[i] ));
}

void setup( const std::string& uri )
{
    PersistentMap map( uri );
    TEST( map.insert( "foo", "bar" ));
    TEST( map.contains( "foo" ));
    TESTINFO( map[ "foo" ] == "bar",
              map[ "foo" ] << " length " << map[ "foo" ].length( ));
    TEST( map[ "bar" ].empty( ));

    TEST( map.insert( "the quick brown fox", "jumped over something" ));
    TESTINFO( map[ "the quick brown fox" ] == "jumped over something",
              map[ "the quick brown fox" ] );

    TEST( map.insert( "hans", std::string( "dampf" )));
    TESTINFO( map[ "hans" ] == "dampf", map[ "hans" ] );

    const bool bValue = true;
    TEST( map.insert( "bValue", bValue ));
    TEST( map.get< bool >( "bValue" ) == bValue );

    const int iValue = 42;
    TEST( map.insert( "iValue", iValue ));
    TEST( map.get< int >( "iValue" ) == iValue );

    insertVector< int >( map );
    insertVector< uint16_t >( map );
    readVector< int >( map );
    readVector< uint16_t >( map );

    std::set< int > set( ints, ints + numInts );
    TEST( map.insert( "std::set< int >", set ));
}

void read( const std::string& uri )
{
    PersistentMap map( uri );
    TEST( map[ "foo" ] == "bar" );
    TEST( map[ "bar" ].empty( ));
    TEST( map.get< bool >( "bValue" ) == true );
    TEST( map.get< int >( "iValue" ) == 42 );

    readVector< int >( map );
    readVector< uint16_t >( map );

    const std::set< int >& set = map.getSet< int >( "std::set< int >" );
    TESTINFO( set.size() ==  numInts, set.size() << " != " << numInts );
    for( size_t i = 0; i < numInts; ++i )
        TESTINFO( set.find( ints[i] ) != set.end(),
                  ints[i] << " not found in set" );
}

void benchmark( const std::string& uri, const size_t queueDepth )
{
    PersistentMap map( uri );
    map.setQueueDepth( queueDepth );
    map.setValueBufferSize(32);

    // Prepare keys
    lunchbox::Strings keys;
    keys.resize( queueDepth + 1 );
    for( uint64_t i = 0; i <= queueDepth; ++i )
        keys[i].assign( reinterpret_cast< char* >( &i ), 8 );

    // write performance
    lunchbox::Clock clock;
    uint64_t i = 0;
    while( clock.getTime64() < loopTime )
    {
        std::string& key = keys[ i % (queueDepth+1) ];
        *reinterpret_cast< uint64_t* >( &key[0] ) = i;
        map.insert( key, key );
        ++i;
    }
    map.flush();
    const float insertTime = clock.getTimef();
    const uint64_t wOps = i;
    TEST( i > queueDepth );

    // read performance
    std::string key;
    key.assign( reinterpret_cast< char* >( &i ), 8 );

    clock.reset();
    for( i = 0; i < queueDepth; ++i ) // prefetch queueDepth keys
    {
        *reinterpret_cast< uint64_t* >( &key[0] ) = i;
        TEST( map.fetch( key ));
    }

    for( ; i < wOps && clock.getTime64() < loopTime; ++i ) // read keys
    {
        *reinterpret_cast< uint64_t* >( &key[0] ) = i - queueDepth;
        map[ key ];

        *reinterpret_cast< uint64_t* >( &key[0] ) = i;
        TEST( map.fetch( key ));
    }

    for( uint64_t j = i - queueDepth; j <= i; ++j ) // drain fetched keys
    {
        *reinterpret_cast< uint64_t* >( &key[0] ) = j;
        map[ key ];
    }

    const float readTime = clock.getTimef();
    std::cout << boost::format(
        "write %6.2f, read %6.2f ops/ms, queue-depth %6i") % (wOps/insertTime)
        % (i/readTime) % queueDepth << std::endl;

    // check contents of store (not all to save time on bigger tests)
    for( uint64_t j = 0; j < wOps && clock.getTime64() < loopTime; ++j )
    {
        *reinterpret_cast< uint64_t* >( &key[0] ) = j;
        TESTINFO( map.get< uint64_t >( key ) == j,
                  j << " = " << map.get< uint64_t >( key ));
    }

    // try to make sure there's nothing outstanding if we messed up i our test.
    map.flush();
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
    catch( const std::runtime_error& )
    {
        return;
    }
    TESTINFO( false, "Missing exception" );
#endif
}

int main( int, char* argv[] )
{
    const bool perfTest LB_UNUSED
        = std::string( argv[0] ).find( "perf_" ) != std::string::npos;
    try
    {
#ifdef __LUNCHBOX_USE_LEVELDB
        setup( "" );
        setup( "leveldb://" );
        setup( "leveldb://persistentMap2.leveldb" );
        read( "" );
        read( "leveldb://" );
        read( "leveldb://persistentMap2.leveldb" );
        if( perfTest )
            benchmark( "leveldb://", 0 );
#endif
#ifdef LUNCHBOX_USE_SKV
        FxLogger_Init( argv[0] );
        setup( "skv://" );
        read( "skv://" );
        if( perfTest )
        {
            benchmark( "skv://", 0 );
            for( size_t i=1; i < 100000; i = i<<1 )
                benchmark( "skv://", i );
        }
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
#ifdef LUNCHBOX_USE_SKV
        if( error.what() !=
            std::string( "skv init failed: SKV_ERRNO_CONN_FAILED" ))
#endif
        {
            TESTINFO( !"exception", error.what( ));
        }
    }

    testGenericFailures();
    testLevelDBFailures();

    return EXIT_SUCCESS;
}
