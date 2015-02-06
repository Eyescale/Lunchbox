
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <sys/stat.h>
#ifdef _MSC_VER
#  include <windows.h>
#elif __APPLE__
#  include <dirent.h>
#  include <mach-o/dyld.h>
#else
#  include <dirent.h>
#  include <limits.h>
#  include <unistd.h>
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

std::string getExecutablePath()
{
    // http://stackoverflow.com/questions/933850
#ifdef _MSC_VER
    char result[MAX_PATH];
    const std::string execPath( result, GetModuleFileName( NULL, result,
                                                           MAX_PATH ));
#elif __APPLE__
    char result[PATH_MAX+1];
    uint32_t size = sizeof(result);
    if( _NSGetExecutablePath( result, &size ) != 0 )
        return std::string();
    const std::string execPath( result );
#else
    char result[PATH_MAX];
    const ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    const std::string execPath( result, count > 0 ? count : 0 );
#endif
    const boost::filesystem::path path( execPath );
    return path.parent_path().string();
}

std::string getLibraryPath()
{
    const std::string& exePath = getExecutablePath();
    if( exePath.empty( ))
        return exePath;

#ifdef _MSC_VER
    return exePath;
#elif __APPLE__
    const boost::filesystem::path path( exePath );

    // foo.app/Contents/MacOS/foo
    if( boost::algorithm::ends_with( exePath, ".app/Contents/MacOS" ))
        return path.parent_path().parent_path().parent_path().parent_path().
                   string() + "/lib";
    return path.parent_path().string() + "/lib";
#else
    const boost::filesystem::path path( exePath );
    return path.parent_path().string() + "/lib";
#endif
}

#define STDSTRING( macro ) std::string( STRINGIFY( macro ))
#define STRINGIFY( foo ) #foo

Strings getLibraryPaths()
{
    Strings paths;
    const std::string& appPath = getLibraryPath();
    if( !appPath.empty( ))
        paths.push_back( appPath );

#ifdef _MSC_VER
    const char* env = ::getenv( "PATH" );
    boost::char_separator< char > separator(";");
    paths.push_back( STDSTRING( CMAKE_INSTALL_PREFIX ) + "/bin" );
#elif __APPLE__
    const char* env = ::getenv( "LD_LIBRARY_PATH" );
    boost::char_separator< char > separator(":");
    paths.push_back( STDSTRING( CMAKE_INSTALL_PREFIX ) + "/lib" );
#else
    const char* env = ::getenv( "DYLD_LIBRARY_PATH" );
    boost::char_separator< char > separator(":");
    paths.push_back( STDSTRING( CMAKE_INSTALL_PREFIX ) + "/lib" );
#endif

    if( !env )
        return paths;

    const boost::tokenizer< boost::char_separator< char > >
        tokens( std::string( env ), separator );
    BOOST_FOREACH( const std::string& token, tokens )
        paths.push_back( token );

    return paths;
}


}
