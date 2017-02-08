
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#include <lunchbox/test.h>

#include <lunchbox/init.h>

int main(const int argc, char** argv)
{
    TEST(lunchbox::init(0, 0));
    TEST(lunchbox::init(argc, argv));
    TEST(lunchbox::exit());
    TEST(lunchbox::exit());
    TEST(!lunchbox::exit());

    TEST(lunchbox::init(argc, argv));
    TEST(lunchbox::exit());
    return EXIT_SUCCESS;
}
