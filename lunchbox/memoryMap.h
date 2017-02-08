
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

#ifndef LUNCHBOX_MEMORYMAP_H
#define LUNCHBOX_MEMORYMAP_H

#include <boost/noncopyable.hpp>
#include <iostream>
#include <lunchbox/api.h>
#include <string>

namespace lunchbox
{
namespace detail
{
class MemoryMap;
}

/**
 * Helper to map a file to a memory address (mmap).
 * @deprecated Use boost::iostreams::mapped_file_source
 *
 * Example: @include tests/memoryMap.cpp
 */
class MemoryMap : public boost::noncopyable
{
public:
    /** Construct a new memory map. @version 1.0 */
    LUNCHBOX_API MemoryMap();

    /**
     * Construct and initialize a new, readonly memory map.
     *
     * @throw std::runtime_error if file can't be mapped.
     * @version 1.7.1 */
    LUNCHBOX_API explicit MemoryMap(const std::string& filename);

    /**
     * Construct and initialize a new, read-write memory map.
     *
     * @throw std::runtime_error if file can't be created.
     * @version 1.9.1
     */
    LUNCHBOX_API MemoryMap(const std::string& filename, const size_t size);

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
    LUNCHBOX_API const void* map(const std::string& filename);

    /**
     * Remap a different file for this memory map.
     *
     * The file is only mapped read-only. An existing map is unmapped.
     *
     * @param filename The filename of the file to map.
     * @return the pointer to the mapped file, or 0 upon error.
     * @version 1.9.1
     */
    LUNCHBOX_API const void* remap(const std::string& filename);

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
    LUNCHBOX_API void* create(const std::string& filename, const size_t size);

    /**
     * Recreate a different writable file for this memory map.
     *
     * The file is only mapped read-write. An existing map is unmapped.
     *
     * @param filename The filename of the file to map.
     * @param size this size of the file.
     * @return the pointer to the mapped file, or nullptr upon error.
     * @version 1.0
     */
    LUNCHBOX_API void* recreate(const std::string& filename, size_t size);

    /**
     * Resize a writeable memory map.
     *
     * The mapping address may change. An existing read-only map will result in
     * an error. On error, the existing map is unmapped.
     *
     * @param size the new size.
     * @return the new mapping address, or nullptr on error.
     * @version 1.16
     */
    LUNCHBOX_API void* resize(size_t size);

    /** Unmap the file. @version 1.0 */
    LUNCHBOX_API void unmap();

    /** @return the pointer to the memory map. @version 1.0 */
    LUNCHBOX_API const void* getAddress() const;

    /** @return the pointer to the memory map. @version 1.9.1 */
    LUNCHBOX_API void* getAddress();

    /** @return the pointer to the memory map. @version 1.9.1 */
    template <class T>
    const T* getAddress() const
    {
        return static_cast<const T*>(getAddress());
    }

    /** @return the pointer to the memory map. @version 1.9.1 */
    template <class T>
    T* getAddress()
    {
        return static_cast<T*>(getAddress());
    }

    /** Access the given element in the map. @version 1.16 */
    template <class T>
    T& get(const size_t i)
    {
        return getAddress<T>()[i];
    }

    /** Access the given element in the map. @version 1.16 */
    template <class T>
    const T& get(const size_t i) const
    {
        return getAddress<T>()[i];
    }

    /** @return the size of the memory map. @version 1.0 */
    LUNCHBOX_API size_t getSize() const;

private:
    detail::MemoryMap* const impl_;
};

inline std::ostream& operator<<(std::ostream& os, const MemoryMap& m)
{
    return os << "MemoryMap at " << m.getAddress() << " size " << m.getSize();
}
}

#endif // LUNCHBOX_MEMORYMAP_H
