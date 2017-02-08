
/* Copyright (c) 2013-2017 Stefan.Eilemann@epfl.ch
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

#define BOOST_TEST_MODULE MemoryMap

#include <boost/test/unit_test.hpp>
#include <lunchbox/memoryMap.h>
#include <lunchbox/types.h>

#define MAP_SIZE LB_10MB
#define STRIDE 23721

BOOST_AUTO_TEST_CASE(write_read)
{
    lunchbox::MemoryMap map("foo.mmap", MAP_SIZE);
    BOOST_CHECK_EQUAL(map.getSize(), MAP_SIZE);

    BOOST_CHECK(map.recreate("foo.mmap", MAP_SIZE / 2));
    uint8_t* writePtr = map.getAddress<uint8_t>();
    BOOST_CHECK(writePtr);

    size_t i = 0;
    for (; i < MAP_SIZE / 2; i += STRIDE)
        writePtr[i] = uint8_t(i);

    BOOST_CHECK(map.resize(MAP_SIZE));
    writePtr = map.getAddress<uint8_t>();
    for (; i < MAP_SIZE; i += STRIDE)
        writePtr[i] = uint8_t(i);
    map.unmap();

    const void* noPtr = map.map("foo.map");
    BOOST_CHECK(!noPtr);
    BOOST_CHECK_EQUAL(map.getSize(), 0);

    BOOST_CHECK(map.map("foo.mmap"));
    BOOST_CHECK(!map.map("foo.mmap"));
    BOOST_CHECK(map.remap("foo.mmap"));
    const uint8_t* readPtr = map.getAddress<uint8_t>();
    BOOST_CHECK(readPtr);
    BOOST_CHECK_EQUAL(map.getSize(), MAP_SIZE);

    for (i = 0; i < MAP_SIZE; i += STRIDE)
        BOOST_CHECK_EQUAL(readPtr[i], uint8_t(i));
}

BOOST_AUTO_TEST_CASE(exceptions)
{
    BOOST_CHECK_THROW(lunchbox::MemoryMap("doesnotexist"), std::runtime_error);
    BOOST_CHECK_THROW(lunchbox::MemoryMap("/doesnotexist", 42),
                      std::runtime_error);
}
