
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

#include <lunchbox/log.h>
#include <lunchbox/debug.h>

#include "mpi.h"

namespace lunchbox
{

MPI * MPI::_instance = 0;

MPI::MPI()
    : _rank( -1 )
    , _size( -1 )
    , _supportedThreads( false )
    , _init( false )
{
}

#ifndef LUNCHBOX_USE_MPI

/** If Lunchbox do not use MPI, set init but not support. */
MPI::MPI( const int , const char ** )
    : _rank( -1 )
    , _size( -1 )
    , _supportedThreads( false )
    , _init( false )
{

    _init = true;
    LBWARN << "Trying to use MPI without support." << std::endl;
}

MPI::~MPI()
{
}

#else

MPI::MPI( const int argc, const char ** argv )
    : _rank( -1 )
    , _size( -1 )
    , _supportedThreads( false )
    , _init( false )
{
	int threadSupportProvided = -1;
	if( MPI_SUCCESS != MPI_Init_thread( (int*) &argc, (char ***) &argv,
                                            MPI_THREAD_MULTIPLE,
                                            &threadSupportProvided ) )
    {
        LBERROR << "Error at initialization MPI library" << std::endl;
        return;
    }

    _init = true;

	switch( threadSupportProvided )
    {
    case MPI_THREAD_SINGLE:
		LBINFO << "MPI_THREAD_SINGLE thread support" << std::endl;
        break;
	case MPI_THREAD_FUNNELED:
		LBINFO << "MPI_THREAD_FUNNELED thread support" << std::endl;
        break;
    case MPI_THREAD_SERIALIZED:
		LBINFO << "MPI_THREAD_SERIALIZED thread support" << std::endl;
        _supportedThreads = true;
        break;
    case MPI_THREAD_MULTIPLE:
		LBINFO << "MPI_THREAD_MULTIPLE thread support" << std::endl;
        _supportedThreads = true;
        break;
    default:
		LBERROR << "Unknown MPI thread support" << std::endl;
	}

    if( MPI_SUCCESS != MPI_Comm_rank( MPI_COMM_WORLD, &_rank ) )
    {
        LBERROR << "Error determining the rank of the calling\
                    process in the communicator." << std::endl;
    }
    if( MPI_SUCCESS != MPI_Comm_size( MPI_COMM_WORLD, &_size ) )
    {
        LBERROR << "Error determining the size of the group\
                    associated with a communicator." << std::endl;
    }

}

MPI::~MPI()
{
    if( _init )
        if( MPI_SUCCESS != MPI_Finalize() )
        {
            LBERROR << "Error at finalizing MPI library" << std::endl;
        }
}

#endif

bool MPI::supportsThreads() const
{
    LBASSERT( _init );
    return _supportedThreads;
}

int MPI::getRank() const
{
    LBASSERT( _supportedThreads );
    return _rank;
}

int MPI::getSize() const
{
    LBASSERT( _supportedThreads );
    return _size;
}

const MPI * MPI::instance(const int argc, const char ** argv)
{
    if( !_instance )
    {
        static MPI instance = MPI( argc, argv );
        _instance = &instance; 
    }

    return _instance;
}

}
