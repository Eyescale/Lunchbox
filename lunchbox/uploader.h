
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

#ifndef LUNCHBOX_UPLOADER_H
#define LUNCHBOX_UPLOADER_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>
#include <lunchbox/thread.h>         // thread-safety macros

namespace lunchbox
{
namespace detail { class Uploader; }

/** A C++ class to handle one uploader plugin instance. */
class Uploader : public boost::noncopyable
{
public:
    /** Construct a new, invalid uploader instance. @version 1.7.1 */
    LUNCHBOX_API Uploader();

    /**
     * Construct a new, named uploader instance.
     *
     * @param from the plugin registry.
     * @param name the name of the uploader.
     * @version 1.7.1
     */
    LUNCHBOX_API Uploader( PluginRegistry& from, const uint32_t name );

    /** Destruct this uploader. @version 1.7.1 */
    LUNCHBOX_API virtual ~Uploader();

    /** @return true if the instance is usable. @version 1.7.1 */
    LUNCHBOX_API bool isGood( const GLEWContext* gl ) const;

    /**
     * @return true if the instance is usable for the given name.
     * @version 1.7.1
     */
    LUNCHBOX_API bool uses( const uint32_t name ) const;

    /**
     * @return true if the uploader supports the given parameters.
     * @version 1.7.1
     */
    LUNCHBOX_API bool supports( const uint32_t externalFormat,
                                const uint32_t internalFormat,
                                const uint64_t capabilities,
                                const GLEWContext* gl ) const;

    /**
     * Find the best uploader in all plugins for the given parameters.
     *
     * This convenience method searches all compressors in all plugins to find
     * the uploader which supports the given parameters and provides the highest
     * speed.
     * @version 1.7.1
     */
    static LUNCHBOX_API uint32_t choose( const PluginRegistry& from,
                                         const uint32_t externalFormat,
                                         const uint32_t internalFormat,
                                         const uint64_t capabilities,
                                         const GLEWContext* gl );

    /** @return the information about the allocated uploader. @version 1.7.1 */
    LUNCHBOX_API const EqCompressorInfo& getInfo() const;

    /**
     * Set up a new, named uploader instance.
     *
     * @param from the plugin registry
     * @param name the name of the uploader
     * @return true on success, false otherwise.
     * @version 1.7.1
     */
    LUNCHBOX_API bool setup( PluginRegistry& from, const uint32_t name );

    /**
     * Set up a new, auto-selected uploader instance.
     * @sa choose() for parameters.
     * @version 1.7.1
     */
    LUNCHBOX_API bool setup( PluginRegistry& from,
                             const uint32_t externalFormat,
                             const uint32_t internalFormat,
                             const uint64_t capabilities,
                             const GLEWContext* gl );

    /** Reset to EQ_COMPRESSOR_NONE. @version 1.7.1 */
    LUNCHBOX_API void clear();

    /**
     * Upload data from cpu to the frame buffer or texture
     *
     * @param buffer data source
     * @param inDims the dimensions of the input data
     * @param flags capability flags for the compression
     * @param outDims the dimensions of the output data
     * @param destination the destination texture name, or 0 for framebuffer
     * @param gl the OpenGL function table
     * @version 1.7.1
     */
    LUNCHBOX_API void upload( const void* buffer, const uint64_t inDims[4],
                              const uint64_t flags, const uint64_t outDims[4],
                              const unsigned destination,
                              const GLEWContext* gl );
private:
    detail::Uploader* const impl_;
    LB_TS_VAR( _thread );
};
}
#endif  // LUNCHBOX_UPLOADER_H
