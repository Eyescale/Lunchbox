
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

#ifndef LUNCHBOX_MPI_H
#define LUNCHBOX_MPI_H

#include <lunchbox/api.h>
#include <boost/shared_ptr.hpp>

namespace lunchbox
{
namespace detail { class MPI; }

/** MPI functionality wrapper. */
class MPI
{
public:
    /**
     * Construct an MPI handler with command line arguments.
     *
     * Calls MPI_Init_thread requesting MPI_THREAD_MULTIPLE, unless MPI is
     * already initialized. Will initialize the thread support to false
     * depending on the obtained thread support. Will signal not to support
     * threads if MPI was initialized externally.
     *
     * NOTE: Be aware that MPI_THREAD_MULTIPLE is only lightly tested and likely
     * still has some bugs. Please, refer the below links:
     * https://www.open-mpi.org/faq/?category=supported-systems#thread-support
     * https://www.open-mpi.org/doc/v1.4/man3/MPI_Init_thread.3.php
     *
     * @version 1.1.1
     */
    LUNCHBOX_API MPI( int argc, char** argv );

    /** Construct a new MPI handler.
     *
     * See argc, argv ctor for details.
     * @version 1.1.1
     */
    LUNCHBOX_API MPI();

    /**
     * Destruct this handler instance.
     *
     * Calls MPI_Finalize if it is the last copy of an instance which has
     * initialized MPI.
     *
     * @version 1.1.1
     */
    LUNCHBOX_API ~MPI();

    /* @return true if the MPI library has multithread
     * support, otherwise return false.
     * @version 1.1.1
     */
    LUNCHBOX_API bool supportsThreads() const;

    /** @return the rank of the process that calls it @version 1.1.1 */
    LUNCHBOX_API int getRank() const;

    /** @return the number of processes involved @version 1.1.1 */
    LUNCHBOX_API int getSize() const;

private:
    boost::shared_ptr< detail::MPI > _impl;
};

}

#endif // LUNCHBOX_MPI_H
