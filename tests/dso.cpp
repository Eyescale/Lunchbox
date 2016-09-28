
/* Copyright (c) 2014-2016, Stefan.Eilemann@epfl.ch
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

#include <lunchbox/test.h>

#include <lunchbox/dso.h>
#include <boost/filesystem.hpp>
#ifdef __linux__
#  include <gnu/lib-names.h>
#endif

#ifdef _WIN32
#  include <lunchbox/os.h>
#  define fork CreateThread
   const std::string forkFun( "CreateThread" );
#else
   const std::string forkFun( "fork" );
#endif

namespace fs = boost::filesystem;

int main( int, char** )
{
    lunchbox::Strings libraries;
#ifdef _WIN32
    libraries.push_back( "Kernel32.dll" );
    libraries.push_back( "Ws2_32.dll" );
#elif defined (Darwin)
    libraries.push_back( "/usr/lib/libc.dylib" );
    libraries.push_back( "/usr/lib/libtermcap.dylib" );
#else
    libraries.push_back( LIBC_SO );
    libraries.push_back( LIBM_SO );
#endif

    lunchbox::DSO one( libraries[0] );
    lunchbox::DSO two( libraries[1] );
    lunchbox::DSO three;

    TEST( one.isOpen( ));
    TEST( two.isOpen( ));
    TEST( !three.isOpen( ));

    TEST( one != two );
    TEST( one != three );
    TEST( three.open( libraries[0] ));
    TEST( one == three );
    TEST( !three.open( libraries[0] ));
    TEST( one == three );

    TEST( one.getFunctionPointer( forkFun ));
    TESTINFO( one.getFunctionPointer( forkFun ) == &fork,
              one.getFunctionPointer( forkFun ) << " != " << (void*)&fork );
    TEST( !one.getFunctionPointer( "fooBar" ));

    one.close();
    TEST( one != two );
    TEST( one != three );
    TEST( !one.getFunctionPointer( forkFun ));

    two.close();
    TEST( one == two );
    TEST( one != three );

    return EXIT_SUCCESS;
}
