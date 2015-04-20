
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

template< class T > void readVector( const PersistentMap& map )
{
    const std::vector< T >& vector =
        map.getVector< T >( typeid( vector ).name( ));
    TESTINFO( vector.size() ==  numInts, vector.size() << " != " << numInts );
    for( size_t i = 0; i < numInts; ++i )
        TEST( vector[ i ] == T( ints[i] ));
}

template< class T > void insertVector( PersistentMap& map, const size_t elems )
{
    std::vector< T > vector;
    for( size_t i = 0; i < elems; ++i )
        vector.push_back( i );
    TEST( map.insert( std::string( "bulk" ) + typeid( vector ).name(), vector ));
}

template< class T > void readVector( const PersistentMap& map, const size_t elems )
{
    const std::vector< T >& vector =
        map.getVector< T >( std::string( "bulk" ) + typeid( vector ).name());
    TESTINFO( vector.size() ==  elems, vector.size() << " != " << elems );
    for( size_t i = 0; i < numInts; ++i )
        TESTINFO( vector[ i ] == T( i ), vector[ i ] << " != " << i );
}

void read( const PersistentMap& map )
{
    const std::set< uint32_t >& bigSet =
        map.getSet< uint32_t >( "std::set< uint32_t >" );
    TEST( bigSet.size() == 1000 );
    for( uint32_t i = 1; i <= 1000; ++i )
        TEST( bigSet.find( i ) != bigSet.end( ));

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

void read( const std::string& uri )
{
    PersistentMap map( uri );
    read( map );
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

    TEST( map.insert( "coffee", 0xC0FFEE ));
    map.setByteswap( true );
    TEST( map.get< unsigned >( "coffee" ) == 0xEEFFC000u );
    map.setByteswap( false );
    TEST( map.get< int >( "coffee" ) == 0xC0FFEE );

    insertVector< int >( map );
    insertVector< uint16_t >( map );
    readVector< int >( map );
    readVector< uint16_t >( map );

    insertVector< int >( map, LB_128KB );
    insertVector< uint16_t >( map, LB_128KB );
    map.fetch( std::string( "bulk" ) + typeid( std::vector< int > ).name( ));
    map.fetch( std::string( "bulk" ) + typeid( std::vector<uint16_t> ).name( ));
    readVector< int >( map, LB_128KB );
    readVector< uint16_t >( map, LB_128KB );

    std::set< int > set( ints, ints + numInts );
    TEST( map.insert( "std::set< int >", set ));

    std::set< uint32_t > bigSet;
    for( uint32_t i = 1; i <= 1000; ++i )
        bigSet.insert( i );
    TEST( map.insert( "std::set< uint32_t >", bigSet ));

    read( map );
}

void benchmark( const std::string& uri, const uint64_t queueDepth )
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
    if( queueDepth == 0 ) // sync read
    {
        for( i = 0; i < wOps && clock.getTime64() < loopTime; ++i ) // read keys
        {
            *reinterpret_cast< uint64_t* >( &key[0] ) = i - queueDepth;
            map[ key ];
        }
    }
    else // fetch + async read
    {
        for( i = 0; i < queueDepth; ++i ) // prefetch queueDepth keys
        {
            *reinterpret_cast< uint64_t* >( &key[0] ) = i;
            TEST( map.fetch( key, 8 ) );
        }

        for( ; i < wOps && clock.getTime64() < loopTime; ++i ) // read keys
        {
            *reinterpret_cast< uint64_t* >( &key[0] ) = i - queueDepth;
            map[ key ];

            *reinterpret_cast< uint64_t* >( &key[0] ) = i;
            TEST( map.fetch( key, 8 ));
        }

        for( uint64_t j = i - queueDepth; j <= i; ++j ) // drain fetched keys
        {
            *reinterpret_cast< uint64_t* >( &key[0] ) = j;
            map[ key ];
        }
    }

    const float readTime = clock.getTimef();
    const size_t rOps = i;

    // fetch performance
    clock.reset();
    for( i = 0; i < wOps && clock.getTime64() < loopTime && i < LB_128KB; ++i )
    {
        *reinterpret_cast< uint64_t* >( &key[0] ) = i;
        TEST( map.fetch( key, 8 ) );
    }
    const float fetchTime = clock.getTimef();
    const size_t fOps = i;

    std::cout << boost::format( "%7.2f, %7.2f, %7.2f, %6i")
        // cppcheck-suppress zerodivcond
        % queueDepth % (rOps/insertTime) % (wOps/readTime) % (fOps/fetchTime)
              << std::endl;

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
    if( perfTest )
        std::cout << "  async,    read,   write,   fetch" << std::endl;

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
