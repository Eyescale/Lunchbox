
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
#define TEST_RUNTIME 240
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
#include <stdexcept>

using lunchbox::PersistentMap;

const int ints[] = { 17, 53, 42, 65535, 32768 };
const size_t numInts = sizeof( ints ) / sizeof( int );

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

void benchmark( const std::string& uri, const size_t queueDepth, float time_per_loop )
{
    PersistentMap map( uri );
    map.setQueueDepth( queueDepth );

    // Prepare keys
    lunchbox::Strings keys;
    keys.resize( queueDepth + 1 );
    for( uint64_t i = 0; i <= queueDepth; ++i )
        keys[i].assign( reinterpret_cast< char* >( &i ), 8 );

    // write performance
    lunchbox::Clock clock;
    uint64_t i = 0;
    while( clock.getTimef() < time_per_loop )
    {
        std::string& key = keys[ i % (queueDepth+1) ];
        *reinterpret_cast< uint64_t* >( &key[0] ) = i;
        map.insert( key, key );

        ++i;
        ++(*reinterpret_cast< uint64_t* >( &key[0] ));
    }

    map.flush();
    const float insertTime = clock.resetTimef();
    const uint64_t wOps = i;
    std::string key1, key2;
    key1.assign( reinterpret_cast< char* >( &i ), 8 );
    key2.assign( reinterpret_cast< char* >( &i ), 8 );

    static char buffer[65536];
    // read performance
    while( i > 0 && clock.getTimef() < time_per_loop )
    {
        *reinterpret_cast< uint64_t* >( &key1[0] ) = i;
        if (queueDepth>0) {
          map.fetch( key1, buffer, 65536 ); // async fetch
          uint64_t prev_i = i+queueDepth;
          if (prev_i<=wOps) {
              *reinterpret_cast< uint64_t* >( &key2[0] ) = prev_i;
              map[key2]; // read the actual value from N fetches ago
          }
        }
        else {
          map[key1]; // read the actual value from N fetches ago
        }
        --i;
    }

    const float readTime = clock.resetTimef();
    const uint64_t rOps = wOps - i;
    std::cout << rOps / readTime << ", " << wOps / insertTime
              << " r+w ops/ms on " << uri << " " << queueDepth << " async writes"
              << std::endl;

    // check contents of store
    for( uint64_t j = 0; j < wOps; ++j )
    {
        key1.assign( reinterpret_cast< char* >( &j ), 8 );
        TESTINFO( map.get< uint64_t >( key1 ) == j,
                  j << " = " << map.get< uint64_t >( key1 ));
    }
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
    float time_per_loop = 500.0;
    const bool perfTest = std::string( argv[0] ).find( "perf_" ) !=
                          std::string::npos;
    try
    {
#ifdef LUNCHBOX_USE_LEVELDB
        setup( "" );
        setup( "leveldb://" );
        setup( "leveldb://persistentMap2.leveldb" );
        read( "" );
        read( "leveldb://" );
        read( "leveldb://persistentMap2.leveldb" );
        if( perfTest )
            benchmark( "leveldb://", 0, time_per_loop );
#endif
#ifdef LUNCHBOX_USE_SKV
        FxLogger_Init( argv[0] );
        setup( "skv://" );
        read( "skv://" );
        if( perfTest )
        {
            benchmark( "skv://", 0, time_per_loop );
            for( size_t i=1; i < 100000; i = i<<1 )
                benchmark( "skv://", i, time_per_loop );
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
