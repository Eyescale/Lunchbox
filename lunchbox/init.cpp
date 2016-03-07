
/* Copyright (c) 2008-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "init.h"

#include "atomic.h"
#include "rng.h"
#include "thread.h"

#include <stdlib.h>
#include <time.h>

namespace lunchbox
{
namespace
{
    static a_int32_t _initialized;
}

bool init( const int argc, char** argv )
{
#ifndef NDEBUG
    LBVERB << "Options: ";
    for( int i = 1; i < argc; ++i )
        LBVERB << argv[i] << ", ";
    LBVERB << std::endl;
#endif

    for( int i = 1; i < argc; ++i )
    {
        // verbose options
        if( std::string( argv[i] ) == "-vv" )
            Log::level += 2;
        else if( std::string( argv[i] ) == "-v" )
            ++Log::level;
    }

    if( ++_initialized > 1 ) // not first
        return true;

    Log::instance().setThreadName( "Main" );

    const time_t now = ::time(0);
#ifdef _WIN32
    char* gmtString = ::ctime( &now );
#else
    char gmtString[32];
    ::ctime_r( &now, gmtString );

    setenv( "AVAHI_COMPAT_NOWARN", "1", 0 ); // get rid of annoying avahi warning
#endif

    LBDEBUG << "Log level " << Log::getLogLevelString() << " topics "
            << Log::topics << " date " << gmtString << std::flush;
    return true;
}

bool exit()
{
    if( --_initialized > 0 ) // not last
        return true;

    Log::reset();

    if( _initialized < 0 )
    {
        LBERROR << "init/exit call mismatch" << std::endl;
        _initialized = 0;
        return false;
    }
    return true;
}

}
