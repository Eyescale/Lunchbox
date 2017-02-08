
/* Copyright (c) 2017, Daniel.Nachbaur@epfl.ch
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

#define BOOST_TEST_MODULE Buffer
#include <boost/test/unit_test.hpp>

#include <lunchbox/buffer.h>

BOOST_AUTO_TEST_CASE(copy_construct_from_empty_buffer)
{
    lunchbox::Bufferb empty;
    lunchbox::Bufferb newBuffer(empty);
    BOOST_CHECK_EQUAL(newBuffer.getData(), empty.getData());
    BOOST_CHECK_EQUAL(newBuffer.getSize(), empty.getSize());
}
