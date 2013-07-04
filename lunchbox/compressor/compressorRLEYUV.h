
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2013, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef LUNCHBOX_PLUGIN_COMPRESSORRLEYUV
#define LUNCHBOX_PLUGIN_COMPRESSORRLEYUV

#include "compressor.h"

namespace lunchbox
{
namespace plugin
{

class CompressorRLEYUV : public Compressor
{
public:
    CompressorRLEYUV(): Compressor() {}
    virtual ~CompressorRLEYUV() {}

    void compress( const void* const inData, const eq_uint64_t nPixels,
                   const bool useAlpha ) override
        { compress( inData, nPixels, useAlpha, false ); }

    static void decompress( const void* const* inData,
                            const eq_uint64_t* const inSizes,
                            const unsigned nInputs, void* const outData,
                            eq_uint64_t* const outDims, const eq_uint64_t flags,
                            void* const );

    static Compressor* getNewCompressor( const unsigned name )
        { return new CompressorRLEYUV; }

protected:
    void compress( const void* const inData, const eq_uint64_t nPixels,
                   const bool useAlpha, const bool swizzle );
};

class CompressorDiffRLEYUV : public CompressorRLEYUV
{
public:
    CompressorDiffRLEYUV() : CompressorRLEYUV() {}
    virtual ~CompressorDiffRLEYUV() {}

    static Compressor* getNewCompressor( const unsigned name  )
        { return new CompressorDiffRLEYUV; }

    void compress( const void* const inData, const eq_uint64_t nPixels,
                   const bool useAlpha ) override
        { CompressorRLEYUV::compress( inData, nPixels, useAlpha, true ); }

    static void decompress( const void* const* inData,
                            const eq_uint64_t* const inSizes,
                            const unsigned nInputs, void* const outData,
                            eq_uint64_t* const outDims, const eq_uint64_t flags,
                            void* const );
};
}
}
#endif // LUNCHBOX_PLUGIN_COMPRESSORRLEYUV
