
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_PLUGIN_H
#define LUNCHBOX_PLUGIN_H

#include <lunchbox/types.h>
#include <lunchbox/dso.h>           // base class
#include <lunchbox/visitorResult.h> // enum
#include <lunchbox/plugins/compressor.h> // GLEW definition

namespace lunchbox
{
namespace detail { class Plugin; }
/** Holder for all functions and information of one compressor plugin. */
class Plugin : public lunchbox::DSO
{
public:
    /** Construct and initialize a new plugin DSO. */
    LUNCHBOX_API Plugin( const std::string& libraryName );

    /** Destruct this plugin handle. */
    LUNCHBOX_API virtual ~Plugin();

    /** @name Plugin function prototypes. */
    //@{
    typedef size_t ( *GetNumCompressors_t ) ();
    typedef void*  ( *NewCompressor_t ) ( const unsigned );
    typedef void   ( *DeleteCompressor_t ) ( void* const );
    typedef void*  ( *NewDecompressor_t ) ( const unsigned );
    typedef void   ( *DeleteDecompressor_t ) ( void* const );
    typedef void   ( *Compress_t ) ( void* const, const unsigned,
                                     void* const, const uint64_t*,
                                     const uint64_t );
    typedef unsigned ( *GetNumResults_t ) ( void* const, const unsigned );
    typedef void   ( *GetResult_t ) ( void* const, const unsigned,
                                      const unsigned, void** const,
                                      uint64_t* const );
    typedef void   ( *Decompress_t ) ( void* const, const unsigned,
                                       const void* const*,
                                       const uint64_t* const,
                                       const unsigned, void* const,
                                       uint64_t* const,
                                       const uint64_t );
    typedef bool ( *IsCompatible_t ) ( const unsigned, const GLEWContext* );
    typedef void ( *Download_t )( void* const, const unsigned,
                                  const GLEWContext*, const uint64_t*,
                                  const unsigned, const uint64_t,
                                  uint64_t*, void** );
    typedef void ( *StartDownload_t )( void* const, const unsigned,
                                       const GLEWContext*, const uint64_t*,
                                       const unsigned, const uint64_t );
    typedef void ( *FinishDownload_t )( void* const, const unsigned,
                                        const GLEWContext*, const uint64_t*,
                                        const uint64_t, uint64_t*, void** );
    typedef void ( *Upload_t )( void* const, const unsigned,
                                const GLEWContext*, const void*,
                                const uint64_t*,
                                const uint64_t, const uint64_t*,
                                const unsigned  );
    //@}

    /** @name Data Access. */
    //@{
    /** @return true if the plugin is usable. */
    LUNCHBOX_API bool isGood() const;

    /** Visit all compressors. @version 1.7.1 */
    LUNCHBOX_API VisitorResult accept( PluginVisitor& visitor );

    /** Visit all compressors. @version 1.7.1 */
    LUNCHBOX_API VisitorResult accept( ConstPluginVisitor& visitor ) const;

    /** @return true if name is found in the plugin. */
    bool implementsType( const uint32_t name ) const;

    /** @return the information for the given compressor, or 0. */
    EqCompressorInfo findInfo( const uint32_t name ) const;

    /** @internal @return all compressor informations. */
    const CompressorInfos& getInfos() const;
    //@}

    /** @name Plugin function pointers. */
    //@{
    /** Get the number of engines found in the plugin. */
    GetNumCompressors_t const getNumCompressors;

    /** Get a new compressor instance.  */
    NewCompressor_t const newCompressor;

    /** Get a new decompressor instance.  */
    NewDecompressor_t const newDecompressor;

    /** Delete the compressor instance.  */
    DeleteCompressor_t const deleteCompressor;

    /** Delete the decompressor instance.  */
    DeleteDecompressor_t const deleteDecompressor;

    /** Compress data. */
    Compress_t const compress;

    /** Decompress data. */
    Decompress_t const decompress;

    /** Get the number of results from the last compression.  */
    GetNumResults_t const getNumResults;

    /** Get the nth result from the last compression.  */
    GetResult_t const getResult;

    /** Check if the transfer plugin can be used. */
    IsCompatible_t const isCompatible;

    /** Download pixel data. */
    Download_t const download;

    /** Upload pixel data. */
    Upload_t const upload;

    /** Start downloading pixel data. */
    StartDownload_t const startDownload;

    /** Start downloading pixel data. */
    FinishDownload_t const finishDownload;
    //@}

private:
    detail::Plugin* const impl_;
};
}
#endif //LUNCHBOX_PLUGIN_H
