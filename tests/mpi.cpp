
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

// need to include mpi.h before stdio.h to work with Intel MPI
#ifdef LUNCHBOX_USE_MPI
#  include <mpi.h>
#endif

#include "test.h"

#include <lunchbox/mpi.h>

int main( int argc, char** argv )
{
    lunchbox::MPI mpi1;
    lunchbox::MPI mpi2( argc, argv );
    lunchbox::MPI mpi3( mpi1 );

    TEST( mpi1.getRank() == mpi2.getRank( ));
    TEST( mpi1.getSize() == mpi2.getSize( ));
    TEST( mpi1.supportsThreads() == mpi3.supportsThreads( ));
    TEST( mpi1.getRank() == mpi3.getRank( ));
    TEST( mpi1.getSize() == mpi3.getSize( ));

#ifdef LUNCHBOX_USE_MPI
    if( mpi1.getRank() == 0 )
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
