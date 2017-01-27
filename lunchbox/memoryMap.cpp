
/* Copyright (c) 2009-2017, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <stdexcept>

namespace lunchbox
{
namespace detail
{
class MemoryMap
{
public:
    MemoryMap() : ptr( nullptr ) , size( 0 ), _map( 0 )
#ifdef _WIN32
    , _file( nullptr )
#endif
     {}

    void* init( const std::string& filename, const size_t size_ )
    {
        if( ptr )
        {
            LBWARN << "File already mapped" << std::endl;
            return nullptr;
        }

        return _init( filename, size_ );
    }

    void unmap()
    {
        if( !ptr )
            return;

        _unmap();
        ptr = nullptr;
        size = 0;
    }

    void* resize( const size_t size_ )
    {
        if( !ptr )
            return nullptr;
        if( size == size_ )
            return ptr;

#ifdef _WIN32
        ::UnmapViewOfFile( ptr );
        ::CloseHandle( _map );
#else
        ::munmap( ptr, size );
#endif

        return _mapFile( size_ );
    }

    void* ptr;
    size_t size;

private:
#ifdef _WIN32
    HANDLE _file;
    void* _map;

    void* _init( const std::string& filename, const size_t size_ )
    {
        // try to open binary file
        const DWORD access = size_ ? GENERIC_READ | GENERIC_WRITE:GENERIC_READ;
        const DWORD create = size_ ? CREATE_ALWAYS : OPEN_EXISTING;
        _file = ::CreateFile( filename.c_str(), access, FILE_SHARE_READ,
                              0, create, FILE_ATTRIBUTE_NORMAL, 0 );
        if( _file == INVALID_HANDLE_VALUE )
        {
            LBWARN << "Can't open " << filename << ": " << sysError <<std::endl;
            return nullptr;
        }
        return _mapFile( size_ );
    }

    void* _mapFile( const size_t size_ )
    {
        if( !_file )
            return nullptr;

        if( size_ )
        {
            ::SetFilePointer( _file, LONG(size), PLONG(&size)+1, FILE_BEGIN );
            ::SetEndOfFile( _file );
        }

        // create a file mapping
        const DWORD mode = size_ ? PAGE_READWRITE : PAGE_READONLY;
        _map = ::CreateFileMapping( _file, 0, mode, 0, 0, 0 );
        if( !_map )
        {
            ::CloseHandle( _file );
            LBWARN << "File mapping failed: " << sysError << std::endl;
            return nullptr;
        }

        // get a view of the mapping
        ptr = ::MapViewOfFile( _map, size_ ? FILE_MAP_WRITE :
                                             FILE_MAP_READ, 0, 0, 0 );

        // get size
        DWORD highSize;
        const DWORD lowSize = ::GetFileSize( _file, &highSize );
        size = lowSize | ( static_cast< uint64_t >( highSize ) << 32 );
        return ptr;
    }

    void _unmap()
    {
        ::UnmapViewOfFile( ptr );
        ::CloseHandle( _map );
        ::CloseHandle( _file );
        _file = nullptr;
        _map = nullptr;
    }

#else
    int _map;

    void* _init( const std::string& filename, const size_t size_ )
    {
        // try to open binary file (and size it)
        const int flags = size_ ? O_RDWR | O_CREAT : O_RDONLY;
        _map = ::open( filename.c_str(), flags, S_IRUSR | S_IWUSR );
        if( _map < 0 )
        {
            LBINFO << "Can't open " << filename << ": " << sysError <<std::endl;
            return nullptr;
        }

        return _mapFile( size_ );
    }

    void* _mapFile( const size_t size_ )
    {
        if( size_ > 0 && ::ftruncate( _map, size_ ) != 0 )
        {
            LBINFO << "Can't resize file: " << sysError << std::endl;
            return nullptr;
        }

        // retrieve file information
        struct stat status;
        ::fstat( _map, &status );

        // create memory mapped file
        size = status.st_size;
        LBASSERTINFO( size_ == 0 || size_ == size, size_ << " != " << size );

        const int mapFlags = size_ ? PROT_READ | PROT_WRITE : PROT_READ;
        ptr = ::mmap( 0, size, mapFlags, MAP_SHARED, _map, 0 );
        if( ptr == MAP_FAILED )
        {
            ::close( _map );
            ptr = nullptr;
            size = 0;
            _map = 0;
        }
        return ptr;
    }

    void _unmap()
    {
        ::munmap( ptr, size );
        ::close( _map );
        _map = 0;
    }
#endif
};
}

MemoryMap::MemoryMap()
    : impl_( new detail::MemoryMap )
{
}

MemoryMap::MemoryMap( const std::string& filename )
    : impl_( new detail::MemoryMap )
{
    if( !map( filename ))
        LBTHROW( std::runtime_error( "Can't map file" ));
}

MemoryMap::MemoryMap( const std::string& filename, const size_t size )
    : impl_( new detail::MemoryMap )
{
    if( !create( filename, size ))
        LBTHROW( std::runtime_error( "Can't create file" ));
}

MemoryMap::~MemoryMap()
{
    unmap();
    delete impl_;
}

const void* MemoryMap::map( const std::string& filename )
{
    return impl_->init( filename, 0 );
}

const void* MemoryMap::remap( const std::string& filename )
{
    unmap();
    return impl_->init( filename, 0 );
}

void* MemoryMap::create( const std::string& filename, const size_t size )
{
    LBASSERT( size > 0 );
    if( size == 0 )
        return nullptr;

    return impl_->init( filename, size );
}

void* MemoryMap::recreate( const std::string& filename, const size_t size )
{
    unmap();
    return create( filename, size );
}

void* MemoryMap::resize( size_t size )
{
    return impl_->resize( size );
}

void MemoryMap::unmap()
{
    impl_->unmap();
}

const void* MemoryMap::getAddress() const
{
    return impl_->ptr;
}

void* MemoryMap::getAddress()
{
    return impl_->ptr;
}

size_t MemoryMap::getSize() const
{
    return impl_->size;
}

}
