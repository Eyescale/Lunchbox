
/* Copyright (c) 2009-2016, Cedric Stalder <cedric.stalder@gmail.com>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
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
#include "memoryMap.h"
#include "os.h"

#include <servus/serializable.h>
#include <servus/uint128_t.h>
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
#ifdef _MSC_VER
    const size_t lastSeparator = filename.find_last_of('\\');
#else
    const size_t lastSeparator = filename.find_last_of('/');
#endif
    if( lastSeparator == std::string::npos )
        return filename;
    // lastSeparator + 1 may be at most equal to filename.size(), which is good
    return filename.substr( lastSeparator + 1 );
}

std::string getDirname( const std::string& filename )
{
#ifdef _MSC_VER
    const size_t lastSeparator = filename.find_last_of('\\');
#else
    const size_t lastSeparator = filename.find_last_of('/');
#endif
    if( lastSeparator == std::string::npos )
        return "./"; // The final separator is always in the output.
    // The separator will be part of the output.
    // If lastSeparator == 0 (e.g. /file-or-dir) it will assume that the rest
    // of the path is a filename.
    return filename.substr( 0, lastSeparator + 1 );
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
    if( count < 0 )
    {
        // Not all UNIX have /proc/self/exe
        LBWARN << "Could not find absolute executable path" << std::endl;
        return "";
    }
    const std::string execPath( result, count );
#endif

    const boost::filesystem::path path( execPath );
#ifdef __APPLE__
    if( boost::algorithm::ends_with( path.parent_path().string(),
                                     "Contents/MacOS" ))
    {
        return
          path.parent_path().parent_path().parent_path().parent_path().string();
    }
#endif
    return path.parent_path().string();
}

std::string getRootPath()
{
    const std::string& exePath = getExecutablePath();
    if( exePath.empty( ))
        return exePath;

    const boost::filesystem::path path( exePath );
#ifdef _MSC_VER
    const Strings buildTypes { "debug", "relwithdebinfo", "release",
                               "minsizerel" };
    std::string buildType( path.stem().string( ));
    std::transform( buildType.begin(), buildType.end(), buildType.begin(),
                    ::tolower );
    if( std::find( buildTypes.begin(), buildTypes.end(),
                   buildType ) != buildTypes.end( ))
    {
        return path.parent_path().parent_path().string();
    }
#endif
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
    paths.push_back( STDSTRING( CMAKE_INSTALL_PREFIX ) + "/bin" );
    const char* env = ::getenv( "PATH" );
#elif __APPLE__
    paths.push_back( STDSTRING( CMAKE_INSTALL_PREFIX ) + "/lib" );
    const char* env = ::getenv( "DYLD_LIBRARY_PATH" );
#else
    paths.push_back( STDSTRING( CMAKE_INSTALL_PREFIX ) + "/lib" );
    const char* env = ::getenv( "LD_LIBRARY_PATH" );
#endif

    if( !env )
        return paths;

    const std::string envString( env );
#ifdef _MSC_VER
    boost::char_separator< char > separator(";");
#else
    boost::char_separator< char > separator(":");
#endif
    const boost::tokenizer< boost::char_separator< char > >
        tokens( envString, separator );
    BOOST_FOREACH( const std::string& token, tokens )
        paths.push_back( token );

    return paths;
}

bool saveBinary( const servus::Serializable& object, const std::string& file )
{
    object.notifyRequested();
    const auto& data = object.toBinary();
    MemoryMap mmap( file, sizeof( uint128_t ) + data.size );
    if( !mmap.getAddress( ))
        return false;
    const uint128_t& id = object.getTypeIdentifier();
    ::memcpy( mmap.getAddress(), &id, sizeof( id ));
    ::memcpy( mmap.getAddress< uint8_t >() + sizeof( id ),
              data.ptr.get(), data.size );
    return true;
}

bool loadBinary( servus::Serializable& object, const std::string& file )
{
    const MemoryMap mmap( file );
    if( !mmap.getAddress() || mmap.getSize() < sizeof( uint128_t ) ||
        *mmap.getAddress< uint128_t >() != object.getTypeIdentifier( ))
    {
        return false;
    }

    object.fromBinary( mmap.getAddress< uint8_t >() + sizeof( uint128_t ),
                       mmap.getSize() - sizeof( uint128_t ));
    object.notifyUpdated();
    return true;
}

bool saveAscii( const servus::Serializable& object, const std::string& file )
{
    object.notifyRequested();
    const std::string& data = object.toJSON();
    MemoryMap mmap( file, data.length( ));
    if( !mmap.getAddress( ))
        return false;
    ::memcpy( mmap.getAddress(), &data[0], data.length( ));
    return true;
}

bool loadAscii( servus::Serializable& object, const std::string& file )
{
    const MemoryMap mmap( file );
    if( !mmap.getAddress( ))
        return false;
    const uint8_t* ptr = mmap.getAddress< uint8_t >();
    object.fromJSON( std::string( ptr, ptr + mmap.getSize( )));
    object.notifyUpdated();
    return true;
}


}
