
/* Copyright (c) 2013-2014, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_COMPRESSORRESULT_H
#define LUNCHBOX_COMPRESSORRESULT_H

#include <lunchbox/array.h> // used inline as CompressorChunk
#include <lunchbox/plugins/compressorTypes.h> // EQ_COMPRESSOR_INVALID

namespace lunchbox
{
/**
 * A structure to hold one compression results.
 *
 * Valid as long as the associated Compressor is valid and has not been used
 * again.
 */
struct CompressorResult
{
    CompressorResult() : compressor( EQ_COMPRESSOR_INVALID ) {}
    CompressorResult( const unsigned n, const CompressorChunks& c )
        : compressor( n ), chunks( c ) {}

    uint64_t getSize() const //!< @return the aggregate size of all chunks
    {
        uint64_t size = 0;
        BOOST_FOREACH( const CompressorChunk& chunk, chunks )
            size += chunk.getNumBytes();
        return size;
    }

    unsigned compressor;
    CompressorChunks chunks;
};
}
#endif  // LUNCHBOX_COMPRESSORRESULT_H
