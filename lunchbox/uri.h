/* Copyright (c) 2013-2015, Ahmet.Bilgili@epfl.ch
 *                        , Stefan.Eilemann@epfl.ch
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
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

#ifndef LUNCHBOX_URI_H
#define LUNCHBOX_URI_H

#include <lunchbox/defines.h>

#ifdef LUNCHBOX_USE_V1_API
#  include <servus/uri.h>

namespace lunchbox
{
    using servus::URI; // In case types.h is not included
}
#else
#  error "This header file may not be included in API version 2"
#endif

#endif // LUNCHBOX_URI_H
