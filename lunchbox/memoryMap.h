
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

#ifndef LUNCHBOX_MEMORYMAP_H
#define LUNCHBOX_MEMORYMAP_H

#include <lunchbox/api.h>
#include <lunchbox/nonCopyable.h>

#include <iostream>
#include <string>

namespace lunchbox
{
/** Helper to map a file to a memory address (mmap) */
class MemoryMap : public NonCopyable
{
public:
    /** Construct a new memory map. @version 1.0 */
    LUNCHBOX_API MemoryMap();

    /** Construct and initialize a new, readonly memory map. @version 1.7.1 */
    LUNCHBOX_API explicit MemoryMap( const std::string& filename );

    /** Construct and initialize a new, read-write memory map. @version 1.9.1 */
    LUNCHBOX_API MemoryMap( const std::string& filename, const size_t size );

    /**
     * Destruct the memory map.
     *
     * Unmaps the file, if it is still mapped.
     * @sa unmap()
     * @version 1.0
     */
    LUNCHBOX_API ~MemoryMap();

    /**
     * Map a file to a memory address.
     *
     * The file is only mapped read-only. The file is automatically unmapped
     * when the memory map is deleted.
     *
     * @param filename The filename of the file to map.
     * @return the pointer to the mapped file, or 0 upon error.
     * @version 1.0
     */
    LUNCHBOX_API const void* map( const std::string& filename );

    /**
     * Remap a different file for this memory map.
     *
     * The file is only mapped read-only. An existing map is unmapped.
     *
     * @param filename The filename of the file to map.
     * @return the pointer to the mapped file, or 0 upon error.
     * @version 1.9.1
     */
    LUNCHBOX_API const void* remap( const std::string& filename );

    /**
     * Create a writable file to a memory address.
     *
     * The file is mapped read-write. An existing file will be overwritten. The
     * file is automatically unmapped when the memory map is deleted.
     *
     * @param filename The filename of the file to map.
     * @param size this size of the file.
     * @return the pointer to the mapped file, or 0 upon error.
     * @version 1.9.1
     */
    LUNCHBOX_API void* create( const std::string& filename, const size_t size );

    /**
     * Recreate a different writable file for this memory map.
     *
     * The file is only mapped read-write. An existing map is unmapped.
     *
     * @param filename The filename of the file to map.
     * @param size this size of the file.
     * @return the pointer to the mapped file, or 0 upon error.
     * @version 1.0
     */
    LUNCHBOX_API void* recreate( const std::string& filename,
                                 const size_t size );

    /** Unmap the file. @version 1.0 */
    LUNCHBOX_API void unmap();

    /** @return the pointer to the memory map. @version 1.0 */
    const void* getAddress() const { return _ptr; }

    /** @return the pointer to the memory map. @version 1.9.1 */
    void* getAddress() { return _ptr; }

    /** @return the pointer to the memory map. @version 1.9.1 */
    template< class T > const T* getAddress() const
        { return static_cast< const T* >( _ptr ); }

    /** @return the pointer to the memory map. @version 1.9.1 */
    template< class T > T* getAddress() { return static_cast< T* >( _ptr ); }

    /** @return the size of the memory map. @version 1.0 */
    size_t getSize() const { return _size; }

private:
#ifdef _WIN32
    void* _map;
#else
    int _fd;
#endif

    void* _ptr;
    size_t _size;

    void* _init( const std::string& filename, const size_t size );
};

inline std::ostream& operator << ( std::ostream& os, const MemoryMap& m )
{
    return os << "MemoryMap at " << m.getAddress() << " size " << m.getSize();
}

}

#endif //LUNCHBOX_MEMORYMAP_H
