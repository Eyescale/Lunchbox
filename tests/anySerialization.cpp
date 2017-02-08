
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "serialize.h"
#include <lunchbox/test.h>

#include <boost/foreach.hpp>

int main(int, char**)
{
    std::list<lunchbox::Any> testValues;
    testValues.push_back(int8_t(-1));
    testValues.push_back(uint8_t(1));
    testValues.push_back(int16_t(-10));
    testValues.push_back(uint16_t(10));
    testValues.push_back(int32_t(-100));
    testValues.push_back(uint32_t(100));
    testValues.push_back(int64_t(-1000));
    testValues.push_back(uint64_t(1000));
    testValues.push_back(bool(false));
    testValues.push_back(float(5.42f));
    testValues.push_back(double(17.56789));
    testValues.push_back(std::string("blablub"));
    testValues.push_back(servus::uint128_t(servus::make_uint128("bla")));
    Foo foo = {42, 1.5f, false, "blablub"};
    testValues.push_back(foo);

    BOOST_FOREACH (const lunchbox::Any& any, testValues)
    {
        textSerializeAndTest(any);
        binarySerializeAndTest(any);
    }

    return EXIT_SUCCESS;
}
