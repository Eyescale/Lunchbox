
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2014, Stefan Eilemann <eile@eyescale.ch>
 *               2010-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_UINT128_H
#define LUNCHBOX_UINT128_H

#include <lunchbox/defines.h>

#ifdef LUNCHBOX_USE_V1_API
#include <servus/uint128_t.h>

namespace lunchbox
{
using servus::uint128_t;
using servus::make_uint128;
using servus::make_UUID;
}
#else
#error "This header file may not be included in API version 2"
#endif

#endif // LUNCHBOX_UINT128_H
