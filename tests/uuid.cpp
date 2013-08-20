
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

// Tests the functionality of universally unique identifiers

#include <test.h>
#include <lunchbox/clock.h>
#include <lunchbox/init.h>
#include <lunchbox/rng.h>
#include <lunchbox/stdExt.h>
#include <lunchbox/thread.h>
#include <lunchbox/uuid.h>

#define N_UUIDS 10000
#define N_THREADS 10

typedef lunchbox::uint128_t uint128_t;
typedef stde::hash_map< uint128_t, bool > TestHash;

void testConvertUint128ToUUID();
void testIncrement();

class Thread : public lunchbox::Thread
{
public:
    virtual void run()
        {
            size_t i = N_UUIDS;

            while( i-- )
            {
                lunchbox::UUID uuid( true );

                TESTINFO( hash.find( uuid ) == hash.end(),
                          "Iteration " << N_UUIDS - i );
                hash[ uuid ] = true;
            }
        }

    TestHash hash;
};

int main( int argc, char **argv )
{
    TEST( lunchbox::init( argc, argv ));

    // basic tests
    lunchbox::UUID id1( true );
    lunchbox::UUID id2( true );

    TEST( id1 != lunchbox::UUID( ));
    TEST( id1 != id2 );

    id1 = id2;
    TEST( id1 == id2 );

    lunchbox::UUID* id3 = new lunchbox::UUID( id1 );
    lunchbox::UUID* id4 = new lunchbox::UUID( true );

    TEST( id1 == *id3 );
    TEST( *id4 != *id3 );

    *id4 = *id3;
    TEST( *id4 == *id3 );

    delete id3;
    delete id4;

    lunchbox::UUID id5, id6;
    TEST( id5 == lunchbox::UUID( ));
    TEST( id5 == id6 );

    const lunchbox::uint128_t& empty = lunchbox::make_uint128( "" );
    const lunchbox::uint128_t& fox = lunchbox::make_uint128(
                               "The quick brown fox jumps over the lazy dog." );
    // Values from http://en.wikipedia.org/wiki/MD5#MD5_hashes
    TEST( empty != fox );
    TESTINFO( empty == uint128_t(0xD41D8CD98F00B204ull, 0xE9800998ECF8427Eull),
              empty );
    TESTINFO( fox == uint128_t(0xE4D909C290D0FB1Cull, 0xA068FFADDF22CBD0ull),
              fox );

    lunchbox::RNG rng;
    uint16_t high = rng.get< uint16_t >();
    int32_t low = rng.get< int32_t >();
    lunchbox::UUID id7( high, low );
    TEST( id7.high() == high );
    TEST( id7.low() == uint64_t( low ));

    // Load tests
    Thread threads[ N_THREADS ];

    lunchbox::Clock clock;
    for( size_t i = 0; i < N_THREADS; ++i )
        threads[ i ].start();
    for( size_t i = 0; i < N_THREADS; ++i )
        threads[ i ].join();

    LBINFO << N_UUIDS * N_THREADS /clock.getTimef()
           << " UUID generations and hash ops / ms" << std::endl;

    TestHash& first = threads[0].hash;
    for( size_t i = 1; i < N_THREADS; ++i )
    {
        TestHash& current = threads[i].hash;

        for( TestHash::const_iterator j = current.begin();
             j != current.end(); ++j )
        {
            lunchbox::UUID uuid( j->first );
            TESTINFO( uuid == j->first, j->first << " = " << uuid );

            std::ostringstream stream;
            stream << uuid;
            uuid = stream.str();
            TESTINFO( uuid == j->first,
                      j->first << " -> " << stream.str() << " -> " << uuid );

            TEST( first.find( uuid ) == first.end( ));
            first[ uuid ] = true;
        }

    }
    testConvertUint128ToUUID();
    testIncrement();
    TEST( lunchbox::exit( ));
    return EXIT_SUCCESS;
}

void testConvertUint128ToUUID()
{
    const uint64_t low = 1212;
    const uint64_t high = 2314;

    uint128_t test128( high, low );
    TEST( test128.low() == low && test128.high() == high );

    lunchbox::UUID testUUID;
    testUUID = test128;
    const uint128_t compare128 = testUUID;
    TEST( compare128 == test128 );
}

void testIncrement()
{
    {
        uint128_t test128( 0, 0 );
        ++test128;
        TEST( test128.high() == 0 && test128.low() == 1 );
        --test128;
        TEST( test128.high() == 0 && test128.low() == 0 );
        test128 = test128 + 1;
        TEST( test128.high() == 0 && test128.low() == 1 );
        test128 = test128 - 1;
        TEST( test128.high() == 0 && test128.low() == 0 );
    }

    {
        uint128_t test128( 0, std::numeric_limits< uint64_t >::max() );
        ++test128;
        TEST( test128.high() == 1 && test128.low() == 0 );
        --test128;
        TEST( test128.high() == 0 &&
              test128.low() == std::numeric_limits< uint64_t >::max() );
        test128 = test128 + 1;
        TEST( test128.high() == 1 && test128.low() == 0 );
        test128 = test128 - 1;
        TEST( test128.high() == 0 &&
              test128.low() == std::numeric_limits< uint64_t >::max() );
    }

    {
        lunchbox::UUID test128( 0, 0 );
        ++test128;
        TEST( test128.high() == 0 && test128.low() == 1 );
        --test128;
        TEST( test128.high() == 0 && test128.low() == 0 );
        test128 = test128 + 1;
        TEST( test128.high() == 0 && test128.low() == 1 );
        test128 = test128 - 1;
        TEST( test128.high() == 0 && test128.low() == 0 );
    }

    {
        lunchbox::UUID test128( 0, std::numeric_limits< uint64_t >::max() );
        ++test128;
        TEST( test128.high() == 1 && test128.low() == 0 );
        --test128;
        TEST( test128.high() == 0 &&
              test128.low() == std::numeric_limits< uint64_t >::max() );
        test128 = test128 + 1;
        TEST( test128.high() == 1 && test128.low() == 0 );
        test128 = test128 - 1;
        TEST( test128.high() == 0 &&
              test128.low() == std::numeric_limits< uint64_t >::max() );
    }
}
