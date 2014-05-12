
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

#ifndef LUNCHBOX_MPI_H
#define LUNCHBOX_MPI_H

#ifdef LUNCHBOX_USE_MPI
#  include <mpi.h>
#endif

namespace lunchbox 
{
    /* MPI library threading support is set by default to MPI_THREAD_SINGLE.
       Lunchbox is a multithreaded library, so, the required level of thread
       support should be MPI_THREAD_SERIALIZED at least.

       NOTE:
       Be aware that MPI_THREAD_MULTIPLE is only lightly tested and likely
       still has some bugs. Please, refer the below links:
       https://www.open-mpi.org/faq/?category=supported-systems#thread-support
       https://www.open-mpi.org/doc/v1.4/man3/MPI_Init_thread.3.php

       MPI_Init_thread and MPI_Finalize:
       Should only be called once.
       Should only be called by a single thread
       Both should be called by the same thread, known as the main thread.

       Here, the MPI library is initialized and requested for
       MPI_THREAD_MULTIPLE level of thread support. To make the library safe, if
       the thread support is not at least MPI_THREAD_SERIALIZED, MPI
       communications will not be allowed.
    */

    /* Base class for MPI functionality.
     * Singlenton pattern, the instance is not a pointer to force destruction.
     */
    class MPI
    {
        public:
            MPI();

            /* @return true if the MPI library has multithread
             * support, otherwise return false.
             * @version 1.1.1
             */
            LUNCHBOX_API bool supportsThreads() const;

            /** @return the rank of the process that calls it @version 1.9 */
            LUNCHBOX_API int getRank() const;

            /** @return the number of processes involved @version 1.9 */
            LUNCHBOX_API int getSize() const;

            /** @return the instance of MPI class. */
            LUNCHBOX_API static const MPI * instance( const int argc = 0,
                                                        const char ** argv = 0);

        private:
            MPI( const int argc, const char ** argv );

            ~MPI();

            int  _rank;
            int  _size;
            bool _supportedThreads;
            bool _init;

            static MPI * _instance;
    };

}

#endif // LUNCHBOX_MPI_H
