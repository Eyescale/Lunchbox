
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2013, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef LUNCHBOX_COMPRESSOR_H
#define LUNCHBOX_COMPRESSOR_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>
#include <lunchbox/thread.h>         // thread-safety macros

namespace lunchbox
{
namespace detail { class Compressor; }

/** A C++ class to handle one compressor plugin instance. */
class Compressor : public NonCopyable
{
public:
    /** Construct a new, invalid compressor instance. @version 1.7.1 */
    LUNCHBOX_API Compressor();

    /**
     * Construct a new, named compressor instance.
     *
     * @param from the plugin registry
     * @param name the name of the compressor
     * @version 1.7.1
     */
    LUNCHBOX_API Compressor( PluginRegistry& from, const uint32_t name );

    /** Destruct the compressor. @version 1.7.1 */
    LUNCHBOX_API virtual ~Compressor();

    /** @return true if the instance is usable. @version 1.7.1 */
    LUNCHBOX_API bool isGood() const;

    /**
     * @return true if the instance is usable for the given name.
     * @version 1.7.1
     */
    LUNCHBOX_API bool uses( const uint32_t name ) const;

    /** @return the information about the allocated instance. @version 1.7.1 */
    LUNCHBOX_API const EqCompressorInfo& getInfo() const;

    /**
     * Find the best compressor in all plugins for the given parameters.
     *
     * This convenience method searches all compressors in all plugins to
     * find the compressor which matches best the given parameters.
     *
     * @param registry the plugin registry to choose from.
     * @param tokenType the structure of the data to compress.
     * @param minQuality minimal quality of the compressed data, with 0 = no
     *                   quality and 1 = full quality, no loss.
     * @param ignoreMSE the most-significant element of a four-element token can
     *                  be ignored, typically the alpha channel of an image.
     * @return the name of the chosen compressor.
     * @version 1.7.1
     */
    static LUNCHBOX_API uint32_t choose( const PluginRegistry& registry,
                                         const uint32_t tokenType,
                                         const float minQuality,
                                         const bool ignoreMSE );

    /**
     * Set up a new, named compressor instance.
     *
     * @param from the plugin registry.
     * @param name the name of the compressor.
     * @return true on success, false otherwise.
     * @version 1.7.1
     */
    LUNCHBOX_API bool setup( PluginRegistry& from, const uint32_t name );

    /**
     * Set up a new, auto-selected compressor instance.
     * @sa choose() for parameters.
     * @version 1.7.1
     */
    LUNCHBOX_API bool setup( PluginRegistry& registry, const uint32_t tokenType,
                             const float minQuality, const bool ignoreMSE );

    /** Reallocate the current instance. @version 1.7.1 */
    LUNCHBOX_API bool realloc();

    /** Reset to EQ_COMPRESSOR_NONE. @version 1.7.1 */
    LUNCHBOX_API void clear();

    /**
     * Compress one-dimensional data.
     *
     * @param in the pointer to the input data.
     * @param inDims the dimensions of the input data
     * @version 1.7.1
     */
    LUNCHBOX_API void compress( void* const in, const uint64_t inDims[2] );

    /**
     * Compress two-dimensional data.
     *
     * @param in the pointer to the input data.
     * @param pvp the dimensions of the input data
     * @param flags capability flags for the compression
     * @version 1.7.1
     */
    LUNCHBOX_API void compress( void* const in, const uint64_t pvp[4],
                                const uint64_t flags );

    /**
     * @return the number of compressed chunks of the last compression.
     * @version 1.7.1
     */
    LUNCHBOX_API unsigned getNumResults() const;

    /**
     * Get one compressed chunk of the last compression.
     *
     * @param i the result index to return.
     * @param out the return value to store the result pointer
     * @param outSize the return value to store the result size in bytes
     * @version 1.7.1
     */
    LUNCHBOX_API void getResult( const unsigned i, void** const out,
                                 uint64_t* const outSize ) const;
private:
    detail::Compressor* const impl_;
    LB_TS_VAR( _thread );
};
}
#endif  // LUNCHBOX_COMPRESSOR_H
