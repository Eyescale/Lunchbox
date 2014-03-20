
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

#ifndef LUNCHBOX_DECOMPRESSOR_H
#define LUNCHBOX_DECOMPRESSOR_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>
#include <lunchbox/thread.h>         // thread-safety macros

namespace lunchbox
{
namespace detail { class Decompressor; }

/**
 * A C++ class to handle one decompressor plugin instance.
 *
 * Example: @include tests/compressor.cpp
 */
class Decompressor : public NonCopyable
{
    typedef detail::Decompressor* const Decompressor::*bool_t;

public:
    /** Construct a new, invalid decompressor instance. @version 1.7.1 */
    LUNCHBOX_API Decompressor();

    /**
     * Construct a new decompressor instance.
     *
     * @param from the plugin registry.
     * @param name the name of the decompressor.
     * @version 1.7.1
     */
    LUNCHBOX_API Decompressor( PluginRegistry& from, const uint32_t name );

    /** Destruct this decompressor. @version 1.7.1 */
    LUNCHBOX_API virtual ~Decompressor();

     /** @return true if the instance is usable. @version 1.7.1 */
    LUNCHBOX_API bool isGood() const;

    /**
     * @return true if the instance is usable, false otherwise.
     * @version 1.9.1
     */
    operator bool_t() const { return isGood() ? &Decompressor::impl_ : 0; }

    /** @return true if the instance is not usable. @version 1.9.1 */
    bool operator ! () const { return !isGood(); }

    /**
     * @return true if the instance is usable for the given name.
     * @version 1.7.1
     */
    LUNCHBOX_API bool uses( const uint32_t name ) const;

    /** @return the information about the allocated instance. @version 1.7.1 */
    LUNCHBOX_API const EqCompressorInfo& getInfo() const;

    /**
     * Set up a new, named decompressor instance.
     *
     * @param from the plugin registry.
     * @param name the name of the decompressor.
     * @return true on success, false otherwise.
     * @version 1.7.1
     */
    LUNCHBOX_API bool setup( PluginRegistry& from, const uint32_t name );

    /** Reset to EQ_COMPRESSOR_NONE. @version 1.7.1 */
    LUNCHBOX_API void clear();

    /**
     * Decompress one-dimensional data.
     *
     * @param in the pointer to an array of input data pointers
     * @param inSizes the array of input data sizes in bytes
     * @param numInputs the number of input data elements
     * @param out the pointer to a pre-allocated buffer for the
     *            uncompressed output result.
     * @param outDim the dimensions of the output data.
     * @version 1.7.1
     */
    LUNCHBOX_API void decompress( const void* const* in,
                                  const uint64_t* const inSizes,
                                  const unsigned numInputs, void* const out,
                                  uint64_t outDim[2] );
    /**
     * Decompress two-dimensional data.
     *
     * @param in the pointer to an array of input data pointers
     * @param inSizes the array of input data sizes in bytes
     * @param numInputs the number of input data elements
     * @param out the pointer to a pre-allocated buffer for the
     *            uncompressed output result.
     * @param pvpOut the dimensions of the output data.
     * @param flags capability flags for the decompression.
     * @version 1.7.1
     */
    LUNCHBOX_API void decompress( const void* const* in,
                                  const uint64_t* const inSizes,
                                  const unsigned numInputs, void* const out,
                                  uint64_t pvpOut[4], const uint64_t flags );

private:
    detail::Decompressor* const impl_;
    LB_TS_VAR( _thread );
};
}
#endif  // LUNCHBOX_DECOMPRESSOR_H
