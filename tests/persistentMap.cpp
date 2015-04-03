
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
#include <deque>
#include <boost/format.hpp>

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
    }
    map.flush();
    const float insertTime = clock.resetTimef();
    const uint64_t wOps = i;
    //std::cout << "Wrote " << wOps << " k/v pairs into database " << std::endl;

    // reserve space in keys
    std::string key1;
    key1.assign( reinterpret_cast< char* >( &i ), 8 );

    // we need space for queueDepth * sizeof values results, for this test, just make a buffer
    std::vector<char> buffer(16*queueDepth, 0);

    std::deque<int> requests;

    // read performance
    // we will not stop after a fixed time as we do not want to leave incomplete requests in the queue
    // fetch i, then N iterations later get it, maintain two counters and stop when all fetches have been read
    clock.resetTimef();
    for (i=0; i<wOps+queueDepth-1; ++i)
    {
        *reinterpret_cast< uint64_t* >( &key1[0] ) = i;
        if (queueDepth==0) {
            map[key1];
        }
        else {
            if (i<wOps) {
                if (queueDepth>0) {
                    //std::cout << "Async fetch for key " << *reinterpret_cast< uint64_t* >( &key1[0] ) << std::endl;
                    uint64_t request = map.fetch( key1, &buffer[(i%(queueDepth+1))*16], 16 ); // async fetch
                    requests.push_back(request);
                }
            }
            if (i>=(queueDepth-1)) {
                uint64_t request = requests.front();
                //uint64_t prefetched_i = i-queueDepth+1;
                //*reinterpret_cast< uint64_t* >( &key2[0] ) = prefetched_i;
                // std::cout << "Async get for key " << *reinterpret_cast< uint64_t* >( &key2[0] ) << " request handle is " << request << std::endl;
                //map[key2]; // read the actual value from N fetches ago
                map.getfetched(request);
                requests.pop_front();
            }
        }
    }
    //std::cout << "Read " << wOps << " k/v pairs from database " << std::endl;
    //
    const float readTime = clock.resetTimef();
    std::cout << boost::format("write, %6.2f, read, %6.2f, ops/ms, queue-depth, %6i") % (wOps/insertTime) % (wOps/readTime) % queueDepth;
    std::cout << std::endl;

    // check contents of store (not all to save time on bigger tests)
    for( uint64_t j = 0; j < wOps && clock.getTimef() < time_per_loop*5.0; ++j )
    {
        key1.assign( reinterpret_cast< char* >( &j ), 8 );
        TESTINFO( map.get< uint64_t >( key1 ) == j,
                  j << " = " << map.get< uint64_t >( key1 ));
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
    float time_per_loop = 200.0;
    const bool perfTest = std::string( argv[0] ).find( "perf_" ) !=
                          std::string::npos;
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
