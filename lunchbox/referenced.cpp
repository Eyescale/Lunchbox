
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "referenced.h"
#include "refPtr.h" // first - debug macros

namespace lunchbox
{
Referenced::Referenced()
    : _refCount(0)
    , _hasBeenDeleted(false)
{
}

Referenced::~Referenced()
{
    LBASSERT(!_hasBeenDeleted);
    _hasBeenDeleted = true;
    LBASSERTINFO(_refCount == 0, "Deleting object with ref count "
                                     << _refCount);
}

void Referenced::notifyFree()
{
    // Don't inline referenced destruction
    delete this;
}
}
