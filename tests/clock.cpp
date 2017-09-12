
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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

#include <lunchbox/clock.h>
#include <lunchbox/sleep.h>

int main(int, char**)
{
    lunchbox::Clock clock;
    lunchbox::Clock copy(clock);

    TEST(clock.getTimef() - copy.getTimef() <= 0.f);
    TEST(clock.getTimef() - copy.getTimef() > -1.f);

    lunchbox::sleep(10.f);
    TEST(clock.resetTimef() > 0.f);
    lunchbox::sleep(1000.f);
    TEST(clock.getTimef() > 990.f);
    TEST(clock.getTimef() < 1100.f);
    TEST(clock.getTimed() < 1100.f);

    return EXIT_SUCCESS;
}
