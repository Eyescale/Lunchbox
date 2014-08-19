
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "file.h"

#include "debug.h"
#include "os.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include <sys/stat.h>
#ifndef _MSC_VER
#  include <dirent.h>
#endif

namespace lunchbox
{
Strings searchDirectory( const std::string& directory,
                         const std::string& pattern )
{
    Strings files;
    const boost::regex regex( pattern );

#ifdef _MSC_VER
    WIN32_FIND_DATA file;
    const std::string search = directory.empty() ? "*.*" : directory + "\\*.*";
    HANDLE hSearch = FindFirstFile( search.c_str(), &file );

    if( hSearch == INVALID_HANDLE_VALUE )
    {
        LBVERB << "Error finding the first file to match " << pattern << " in "
               << directory << std::endl;
        FindClose( hSearch );
        return files;
    }

    if( boost::regex_match( file.cFileName, regex ))
        files.push_back( file.cFileName );
    while( FindNextFile( hSearch, &file ))
    {
        if( boost::regex_match( file.cFileName, regex ))
            files.push_back( file.cFileName );
    }
    FindClose( hSearch );

#else

    DIR* dir = opendir( directory.c_str() );
    if( !dir )
    {
        LBVERB << "Can't open directory " << directory << std::endl;
        return files;
    }

    struct dirent* entry;

    while(( entry = readdir( dir )) != 0 )
    {
        const std::string candidate( entry->d_name );
        if( boost::regex_match( candidate, regex ))
            files.push_back( entry->d_name );
    }

    closedir(dir);
#endif
    return files;
}

Strings expandWildcard( const std::string& directory,
                        const std::string& wildcard )
{
    std::string pattern( wildcard );
    boost::replace_all( pattern, ".", "\\." );
    boost::replace_all( pattern, "*", ".*" );
    boost::replace_all( pattern, "/", "\\/" );

    return searchDirectory( directory, "^" + pattern + "$" );
}

std::string getFilename( const std::string& filename )
{
    size_t lastSeparator = 0;
    const size_t length = filename.length();

    for( size_t i = 0; i < length; ++i )
        if( filename[ i ] == '/' || filename[i] == '\\' )
            lastSeparator = i+1;

    return lastSeparator == 0 ? filename :
                                filename.substr( lastSeparator, length );
}

std::string getDirname( const std::string& filename )
{
    size_t lastSeparator = 0;
    const size_t length = filename.length();

    for( size_t i = 0; i < length; ++i )
        if( filename[ i ] == '/' || filename[i] == '\\' )
            lastSeparator = i+1;

    return lastSeparator == 0 ? "." : filename.substr( 0, lastSeparator );
}

}
