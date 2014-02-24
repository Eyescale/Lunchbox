
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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

#include "test.h"

#include <lunchbox/dso.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main( int, char** )
{
    lunchbox::Strings libraries;
#ifdef _WIN32
    libraries.push_back( "TODO" );
#elif defined (Darwin)
    libraries.push_back( "/usr/lib/libc.dylib" );
    libraries.push_back( "/usr/lib/libtermcap.dylib" );
#else
    const fs::path lib1 ( "/usr/lib/libc.so" );
    if( is_regular_file( lib1 ))
        libraries.push_back( lib1.string( ));
    const fs::path lib2 ( "/lib/x86_64-linux-gnu/libpthread.so.0" );
    if( is_regular_file( lib2 ))
        libraries.push_back( lib2.string( ));
    const fs::path lib3 ("/usr/lib/libtermcap.so" );
    if( is_regular_file( lib3 ))
        libraries.push_back( lib3.string( ));
    const fs::path lib4 ( "/usr/lib/x86_64-linux-gnu/libtinfo.so" );
    if( is_regular_file( lib4 ))
        libraries.push_back( lib4.string( ));
#endif
    TEST( libraries.size() > 1 )

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

    TEST( one.getFunctionPointer( "fork" ));
    TEST( one.getFunctionPointer( "fork" ) == &fork );
    TEST( !one.getFunctionPointer( "fooBar" ));

    one.close();
    TEST( one != two );
    TEST( one != three );
    TEST( !one.getFunctionPointer( "fork" ));

    two.close();
    TEST( one == two );
    TEST( one != three );

    return EXIT_SUCCESS;
}
