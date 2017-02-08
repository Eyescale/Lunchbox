
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

#include <lunchbox/test.h>

#include <lunchbox/any.h>

int main(int, char**)
{
    lunchbox::Any any;
    lunchbox::Any otherAny;
    TEST(any.empty());
    TEST(any.type() == typeid(void));
    TEST(any == otherAny);

    any = 5;
    otherAny = any;
    TEST(lunchbox::any_cast<int>(any) == 5);
    TEST(any.type() == typeid(int));
    TEST(any == otherAny);

    any = 42;
    otherAny = 42;
    TEST(lunchbox::any_cast<int>(any) == 42);
    TEST(any == otherAny);

    any = std::string("blablub");
    TEST(lunchbox::any_cast<std::string>(any) == "blablub");
    TEST(any.type() == typeid(std::string));
    TEST(any != otherAny);

    try
    {
        TEST(lunchbox::any_cast<int>(any) != 42);
    }
    catch (const lunchbox::bad_any_cast&)
    {
    }

    return EXIT_SUCCESS;
}
