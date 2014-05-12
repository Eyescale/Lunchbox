
/* Copyright (c) 2014, Carlos Duelo <cduelo@cesvima.upm.es>
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

#include "test.h"

#include <lunchbox/mpi.h>

int main( int, char** )
{
    if( !lunchbox::MPI::instance()->supportsThreads() )
        return EXIT_SUCCESS;

    #ifdef LUNCHBOX_USE_MPI
    if( lunchbox::MPI::instance()->getRank() == 0 )
    {
        int i = 58;
        MPI_Bcast( &i, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
    {
        int i = -1;
        MPI_Bcast( &i, 1, MPI_INT, 0, MPI_COMM_WORLD);
        TEST( i == 58 );
    }
    #endif

    return EXIT_SUCCESS;
}
