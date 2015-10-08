
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_OMP_H
#define LUNCHBOX_OMP_H

#include <lunchbox/api.h>

#ifdef LUNCHBOX_USE_OPENMP
#  include <omp.h>
#endif

namespace lunchbox
{
/**
 * Base class for OpenMP functionality.
 *
 * Implements dummy calls if the compiler does not support OpenMP
 */
class OMP
{
public:
    /**
     * @return the number of threads used in a parallel region.
     * @version 1.0
     */
    LUNCHBOX_API static unsigned getNThreads();

    /** Set the maximum number of threads. @version 1.12 */
    LUNCHBOX_API static void setNThreads( unsigned nThreads );

    /**
     * @return the id of the current OpenMP thread, or 0 if OpenMP is not used.
     * @version 1.11
     */
    LUNCHBOX_API static int getThreadNum();
};
}

#endif //LUNCHBOX_OMP_H
