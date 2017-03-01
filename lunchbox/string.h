
/* Copyright (c) 2017, Stefan.Eilemann@epfl.ch
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

#pragma once
#include <string>

namespace lunchbox
{
namespace string
{
/**
 * Prepend each line of the given input with the given text.
 *
 * @param input the input string to add the prepended text
 * @param text text to prepend
 * @return the prepended string
 * @version 1.16
 */
inline std::string prepend(const std::string& input, const std::string& text)
{
    std::string output;
    size_t pos = 0;
    for (size_t nextPos = input.find('\n', pos); nextPos != std::string::npos;
         nextPos = input.find('\n', pos))
    {
        output += text + input.substr(pos, nextPos - pos + 1);
        pos = nextPos + 1;
    }
    output += text + input.substr(pos, std::string::npos);
    return output;
}
}
}
