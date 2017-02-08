
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <lunchbox/result.h>
#include <lunchbox/test.h>

int main(int, char**)
{
    const lunchbox::Result success(lunchbox::Result::SUCCESS);
    const lunchbox::Result failure(lunchbox::Result::SUCCESS + 1);

    TEST(success);
    TEST(!failure);

    if (!success)
        TEST(false)

    if (failure)
        TEST(false)

    return EXIT_SUCCESS;
}
