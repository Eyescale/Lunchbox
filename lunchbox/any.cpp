
/* Copyright (c) 2012, Daniel Nachbaur <daniel.nachbaur@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#include "any.h"

#include <cstdio>

#ifdef WIN32
#define snprintf _snprintf_s
#endif

namespace lunchbox
{
Any::Any()
    : content()
{
}

Any::Any(const Any& other)
    : content(other.content ? other.content->clone() : 0)
{
}

Any::~Any()
{
}

Any& Any::swap(Any& rhs)
{
    std::swap(content, rhs.content);
    return *this;
}

Any& Any::operator=(Any rhs)
{
    rhs.swap(*this);
    return *this;
}

bool Any::empty() const
{
    return !content;
}

const std::type_info& Any::type() const
{
    return content ? content->type() : typeid(void);
}

bool Any::operator==(const Any& rhs) const
{
    if ((this == &rhs) || (empty() && rhs.empty()))
        return true;

    if (empty() != rhs.empty() || type() != rhs.type())
        return false;

    return *content == *rhs.content;
}

bad_any_cast::bad_any_cast(const std::string& from, const std::string& to)
{
    snprintf(data, 256,
             "boost::bad_any_cast: failed conversion from %s to %s\n",
             from.c_str(), to.c_str());
    data[255] = 0;
}
}
