
/*
 * Copyright (c) 2010, Eyescale Software GmbH <info@eyescale.ch>
 *               2013, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_PLUGIN_COMPRESSORTURBOJPEG
#define LUNCHBOX_PLUGIN_COMPRESSORTURBOJPEG

#include "compressor.h"
#include <turbojpeg.h>
#undef max

namespace lunchbox
{
namespace plugin
{

class CompressorTurboJPEG : public Compressor
{
public:
    explicit CompressorTurboJPEG( const unsigned name );
    virtual ~CompressorTurboJPEG();

    void compress( const void* const inData, const eq_uint64_t* inDims,
                   const eq_uint64_t flags ) override;

    static void decompress( const void* const* inData,
                            const eq_uint64_t* const inSizes,
                            const unsigned nInputs, void* const outData,
                            eq_uint64_t* const outDims, const eq_uint64_t flags,
                            void* const );

    static Compressor* getNewCompressor( const unsigned name )
        { return new CompressorTurboJPEG( name ); }

private:
    eq_uint64_t _quality;
    eq_uint64_t _tokenSize;
    eq_uint64_t _flags;

    void* _encoder;
    void* _decoder;

    void _decompress( const void* const* inData, const eq_uint64_t inSize,
                      const unsigned nInputs, void* const outData,
                      eq_uint64_t* const outDims, const bool useAlpha );
    void _extractAlpha( const unsigned char* inData,
                        const eq_uint64_t nPixels );
    void _addAlpha( const void* const inAlpha, unsigned* out,
                    const eq_uint64_t nPixels ) const;
};

}
}
#endif  // LUNCHBOX_PLUGIN_COMPRESSORTURBOJPEG
