
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

#include <lunchbox/api.h>
#include <lunchbox/types.h>

namespace lunchbox
{
namespace term
{

/** Terminal sizes in width, height. @version 1.16 */
using size = std::pair< uint32_t, uint32_t >;

/**
 * @return the width and height of the shell running the program.
 * @version 1.16
 */
LUNCHBOX_API size getSize();
}
}
