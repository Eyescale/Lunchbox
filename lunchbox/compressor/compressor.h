
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_PLUGIN_COMPRESSOR
#define LUNCHBOX_PLUGIN_COMPRESSOR

#include <lunchbox/plugins/compressor.h>

#include <lunchbox/buffer.h>
#include <lunchbox/plugin.h>
#include <vector>

/**
 * @file lunchbox/compressor/compressor.h
 *
 * Compression plugins provided with Lunchbox.
 */
namespace lunchbox
{
namespace plugin
{
class Compressor
{
public:
    typedef void        ( *GetInfo_t )( EqCompressorInfo* const );
    typedef Compressor* ( *NewCompressor_t )( const unsigned );
    typedef void        ( *Decompress_t )( const void* const*,
                                           const eq_uint64_t* const,
                                           const unsigned, void* const,
                                           eq_uint64_t* const,
                                           const eq_uint64_t, void* const );
    typedef bool        ( *IsCompatible_t )( const GLEWContext* );
    struct Functions
    {
        Functions( const unsigned name, GetInfo_t getInfo,
                   NewCompressor_t newCompressor,
                   NewCompressor_t newDecompressor,
                   Decompress_t decompress, IsCompatible_t isCompatible );

        unsigned name;
        GetInfo_t getInfo;
        NewCompressor_t newCompressor;
        NewCompressor_t newDecompressor;
        Decompress_t decompress;
        IsCompatible_t isCompatible;
    };

    /** Construct a new compressor. */
    Compressor();
    virtual ~Compressor();

    /**
     * Compress data.
     *
     * @param inData data to compress.
     * @param inDims input dimensions.
     * @param flags compression flags.
     */
    virtual void compress( const void* const inData, const eq_uint64_t* inDims,
                           const eq_uint64_t flags );

    /**
     * Compress data.
     *
     * @param inData data to compress.
     * @param nPixels number data to compress.
     * @param useAlpha use alpha channel in compression.
     */
    virtual void compress( const void* const inData LB_UNUSED,
                           const eq_uint64_t nPixels LB_UNUSED,
                           const bool useAlpha LB_UNUSED ) { LBDONTCALL; };

    typedef lunchbox::Bufferb Result;
    typedef std::vector< Result* > ResultVector;

    /** @return the vector containing the result data. */
    const ResultVector& getResults() const { return _results; }

    /** @return the number of result items produced. */
    unsigned getNResults() const { return _nResults; }

    /**
     * Transfer frame buffer data into main memory.
     *
     * @param glewContext the initialized GLEW context describing
     *                    corresponding to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param source texture name to process.
     * @param flags capability flags for the compression (see description).
     * @param outDims the dimensions of the output data (see description).
     * @param out the pointer to the output data.
     */
    virtual void download( const GLEWContext* glewContext LB_UNUSED,
                           const eq_uint64_t  inDims[4] LB_UNUSED,
                           const unsigned     source LB_UNUSED,
                           const eq_uint64_t  flags LB_UNUSED,
                           eq_uint64_t        outDims[4] LB_UNUSED,
                           void**             out LB_UNUSED ) { LBDONTCALL; }

    /**
     * Transfer data from main memory into GPU memory.
     *
     * @param glewContext the initialized GLEW context describing
     *                    corresponding to the current OpenGL context.
     * @param buffer the datas input.
     * @param inDims the dimension of data in the frame buffer.
     * @param flags capability flags for the compression.
     * @param outDims the result data size
     * @param destination the destination texture name.
     */
    virtual void upload( const GLEWContext* glewContext LB_UNUSED,
                         const void*        buffer LB_UNUSED,
                         const eq_uint64_t  inDims[4] LB_UNUSED,
                         const eq_uint64_t  flags LB_UNUSED,
                         const eq_uint64_t  outDims[4] LB_UNUSED,
                         const unsigned destination LB_UNUSED ) { LBDONTCALL; }

    /**
     * Start transferring frame buffer data into main memory.
     *
     * @param glewContext the initialized GLEW context describing
     *                    corresponding to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param source texture name, if EQ_COMPRESSOR_USE_TEXTURE_2D or
     *               EQ_COMPRESSOR_USE_TEXTURE_RECT is set.
     * @param flags capability flags for the compression (see description).
     * @version 4
     */
    virtual void startDownload( const GLEWContext* glewContext LB_UNUSED,
                                const eq_uint64_t  inDims[4] LB_UNUSED,
                                const unsigned     source LB_UNUSED,
                                const eq_uint64_t flags LB_UNUSED ) {LBDONTCALL}


    /**
     * Finish transferring frame buffer data into main memory.
     *
     * @param glewContext the initialized GLEW context describing
     *                    corresponding to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param source texture name, if EQ_COMPRESSOR_USE_TEXTURE_2D or
     *               EQ_COMPRESSOR_USE_TEXTURE_RECT is set.
     * @param flags capability flags for the compression (see description).
     * @param outDims the dimensions of the output data (see description).
     * @param out the pointer to the output data.
     * @version 4
     */
    virtual void finishDownload( const GLEWContext* glewContext LB_UNUSED,
                                 const eq_uint64_t  inDims[4] LB_UNUSED,
                                 const unsigned     source LB_UNUSED,
                                 const eq_uint64_t  flags LB_UNUSED,
                                 eq_uint64_t        outDims[4] LB_UNUSED,
                                 void** out LB_UNUSED ) { LBDONTCALL; }

    /** @internal Register a new plugin engine. */
    static void registerEngine( const Functions& functions );

    /** Convenience function for instance-less decompressor allocation. */
    static Compressor* getNewDecompressor( const unsigned /*name*/ ){ return 0;}

protected:
    ResultVector _results;  //!< The compressed data
    unsigned _nResults;     //!< Number of elements used in _results

};
}
}

#endif // LUNCHBOX_PLUGIN_COMPRESSOR
