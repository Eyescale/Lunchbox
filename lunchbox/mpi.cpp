
/* Copyright (c) 2014, Carlos Duelo <cduelo@cesvima.upm.es>
 *                     Stefan.Eilemann@epfl.ch
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

#include "mpi.h"

#include <lunchbox/log.h>
#include <lunchbox/debug.h>


namespace lunchbox
{
namespace
{
    bool _supportsThreads = false;
}

namespace detail
{
class MPI
{
public:
    MPI( int* argc LB_UNUSED, char*** argv LB_UNUSED )
        : rank( -1 )
        , size( -1 )
        , initCalled( false )
    {
#ifdef LUNCHBOX_USE_MPI
        int initialized = false;
        MPI_Initialized( &initialized );
        if( !initialized )
        {
            int threadLevel = -1;
            if( MPI_SUCCESS != MPI_Init_thread( argc, argv,
                                                MPI_THREAD_MULTIPLE,
                                                &threadLevel ))
            {
                LBERROR << "MPI_Init_thread failed" << std::endl;
                return;
            }

            initCalled = true;

            switch( threadLevel )
            {
            case MPI_THREAD_SINGLE:
                LBVERB << "MPI_THREAD_SINGLE thread support" << std::endl;
                break;
            case MPI_THREAD_FUNNELED:
                LBVERB << "MPI_THREAD_FUNNELED thread support" << std::endl;
                break;
            case MPI_THREAD_SERIALIZED:
                LBVERB << "MPI_THREAD_SERIALIZED thread support" << std::endl;
                _supportsThreads = true;
                break;
            case MPI_THREAD_MULTIPLE:
                LBVERB << "MPI_THREAD_MULTIPLE thread support" << std::endl;
                _supportsThreads = true;
                break;
            default:
                LBERROR << "Unknown MPI thread support" << std::endl;
            }
        }

        if( MPI_SUCCESS != MPI_Comm_rank( MPI_COMM_WORLD, &rank ) )
            LBERROR << "MPI_Comm_rank failed" << std::endl;

        if( MPI_SUCCESS != MPI_Comm_size( MPI_COMM_WORLD, &size ) )
            LBERROR << "MPI_Comm_size failed" << std::endl;
#endif
    }

    ~MPI()
    {
#ifdef LUNCHBOX_USE_MPI
        if( !initCalled )
            return;

        if( MPI_SUCCESS != MPI_Finalize() )
            LBERROR << "MPI_Finalize failed" << std::endl;
        _supportsThreads = false;
#endif
    }

    int  rank;
    int  size;
    bool initCalled;

};
}

MPI::MPI()
    : _impl( new detail::MPI( 0, 0 ))
{}

MPI::MPI( int& argc, char**& argv )
    : _impl( new detail::MPI( &argc, &argv ))
{}

MPI::~MPI()
{
}


bool MPI::supportsThreads() const
{
    return _supportsThreads;
}

int MPI::getRank() const
{
    return _impl->rank;
}

int MPI::getSize() const
{
    return _impl->size;
}

}
