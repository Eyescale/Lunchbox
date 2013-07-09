
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "memoryMap.h"

#include "debug.h"
#include "os.h"
#include "types.h"

#include <fcntl.h>
#include <sys/stat.h>
#ifndef _WIN32
#  include <sys/mman.h>
#  include <unistd.h>
#endif

namespace lunchbox
{

MemoryMap::MemoryMap()
#ifdef _WIN32
        : _map( 0 )
#else
        : _fd( 0 )
#endif
        , _ptr( 0 )
        , _size( 0 )
{
}

MemoryMap::MemoryMap( const std::string& filename )
#ifdef _WIN32
        : _map( 0 )
#else
        : _fd( 0 )
#endif
        , _ptr( 0 )
        , _size( 0 )
{
    map( filename );
}

MemoryMap::MemoryMap( const std::string& filename, const size_t size )
#ifdef _WIN32
        : _map( 0 )
#else
        : _fd( 0 )
#endif
        , _ptr( 0 )
        , _size( 0 )
{
    create( filename, size );
}

MemoryMap::~MemoryMap()
{
    if( _ptr )
        unmap();
}

const void* MemoryMap::map( const std::string& filename )
{
    return _init( filename, 0 );
}

void* MemoryMap::create( const std::string& filename, const size_t size )
{
    LBASSERT( size > 0 );
    if( size == 0 )
        return 0;

    return _init( filename, size );
}

void* MemoryMap::_init( const std::string& filename, const size_t size )
{
    if( _ptr )
    {
        LBWARN << "File already mapped" << std::endl;
        return 0;
    }

#ifdef _WIN32
    // try to open binary file (and size it)
    const DWORD access = size ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
    const DWORD create = size ? CREATE_ALWAYS : OPEN_EXISTING;
    HANDLE file = CreateFile( filename.c_str(), access, FILE_SHARE_READ,
                              0, create, FILE_ATTRIBUTE_NORMAL, 0 );
    if( file == INVALID_HANDLE_VALUE )
    {
        LBWARN << "Can't open " << filename << ": " << sysError << std::endl;
        return 0;
    }

    if( size )
    {
        SetFilePointer( file, size, 0, FILE_BEGIN );
        SetEndOfFile( file );
    }

    // create a file mapping
    const DWORD mode = size ? PAGE_READWRITE : PAGE_READONLY;
    _map = CreateFileMapping( file, 0, mode, 0, 0, 0 );
    if( !_map )
    {
        LBWARN << "File mapping failed: " << sysError << std::endl;
        return 0;
    }

    // get a view of the mapping
    _ptr = MapViewOfFile( _map, FILE_MAP_READ, 0, 0, 0 );

    // get size
    DWORD highSize;
    const DWORD lowSize = GetFileSize( file, &highSize );
    _size = lowSize | ( static_cast< uint64_t >( highSize ) << 32 );
    LBASSERT( size == 0 || size == _size );

    CloseHandle( file );
#else // POSIX

    // try to open binary file (and size it)
    const int flags = size ? O_RDWR | O_CREAT : O_RDONLY;
    _fd = open( filename.c_str(), flags, S_IRUSR | S_IWUSR );
    if( _fd < 0 )
    {
        LBINFO << "Can't open " << filename << ": " << sysError << std::endl;
        return 0;
    }

    if( size > 0 && ::ftruncate( _fd, size ) != 0 )
    {
        LBINFO << "Can't resize " << filename << ": " << sysError << std::endl;
        return 0;
    }

    // retrieving file information
    struct stat status;
    fstat( _fd, &status );

    // create memory mapped file
    _size = status.st_size;
    LBASSERT( size == 0 || size == _size );

    const int mapFlags = size ? PROT_READ | PROT_WRITE : PROT_READ;
    _ptr = mmap( 0, _size, mapFlags, MAP_SHARED, _fd, 0 );
    if( _ptr == MAP_FAILED )
    {
        close( _fd );
        _ptr = 0;
        _size = 0;
        _fd = 0;
    }
#endif

    return _ptr;
}

void MemoryMap::unmap()
{
    if( !_ptr )
    {
        LBWARN << "File not mapped" << std::endl;
        return;
    }

#ifdef _WIN32
    UnmapViewOfFile( _ptr );
    CloseHandle( _map );

    _ptr = 0;
    _size = 0;
    _map = 0;
#else
    munmap( _ptr, _size );
    close( _fd );

    _ptr = 0;
    _size = 0;
    _fd = 0;
#endif
}

}
