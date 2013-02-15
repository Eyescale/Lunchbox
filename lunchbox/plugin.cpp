
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This file is part of Collage <https://github.com/Eyescale/Collage>
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

#include "plugin.h"

#include "compressorInfo.h"
#include "debug.h"
#include "log.h"
#include "pluginRegistry.h"
#include <cmath>

namespace lunchbox
{
typedef void ( *GetInfo_t ) ( const size_t, EqCompressorInfo* const );

Plugin::Plugin( const std::string& libraryName )
    : dso_( libraryName )
    , getNumCompressors( dso_.getFunctionPointer< GetNumCompressors_t >(
                             "EqCompressorGetNumCompressors" ))
    , newCompressor( dso_.getFunctionPointer< NewCompressor_t >(
                         "EqCompressorNewCompressor" ))
    , newDecompressor( dso_.getFunctionPointer< NewDecompressor_t >(
                           "EqCompressorNewDecompressor" ))
    , deleteCompressor( dso_.getFunctionPointer< DeleteCompressor_t >(
                            "EqCompressorDeleteCompressor" ))
    , deleteDecompressor( dso_.getFunctionPointer< DeleteDecompressor_t >(
                              "EqCompressorDeleteDecompressor" ))
    , compress( dso_.getFunctionPointer< Compress_t >( "EqCompressorCompress" ))
    , decompress( dso_.getFunctionPointer< Decompress_t >(
                      "EqCompressorDecompress" ))
    , getNumResults( dso_.getFunctionPointer< GetNumResults_t >(
                         "EqCompressorGetNumResults" ))
    , getResult( dso_.getFunctionPointer< GetResult_t >(
                     "EqCompressorGetResult" ))
    , isCompatible( dso_.getFunctionPointer< IsCompatible_t >(
                        "EqCompressorIsCompatible" ))
    , download( dso_.getFunctionPointer< Download_t >( "EqCompressorDownload" ))
    , upload( dso_.getFunctionPointer< Upload_t >( "EqCompressorUpload" ))
    , startDownload( dso_.getFunctionPointer< StartDownload_t >(
                         "EqCompressorStartDownload" ))
    , finishDownload( dso_.getFunctionPointer< FinishDownload_t >(
                          "EqCompressorFinishDownload" ))
{
    if( !dso_.isOpen( ))
        return;

    const GetInfo_t getInfo =
        dso_.getFunctionPointer< GetInfo_t >( "EqCompressorGetInfo" );

    const bool hasBase = newDecompressor && newCompressor && deleteCompressor &&
                         deleteDecompressor && getInfo && getNumCompressors &&
                         getInfo;
    const bool hasCPU = getResult && getNumResults && decompress && compress;
    const bool hasGPU = isCompatible && download && upload;
    if( !hasBase || ( !hasCPU && !hasGPU ))
    {
        LBWARN << "Initializing compression DSO " << libraryName
               << " failed, at least one entry point missing" << std::endl;
        return;
    }

    const size_t nCompressors = getNumCompressors();
    if( nCompressors == 0 )
    {
        LBWARN << "Initializing compression DSO " << libraryName
               << " failed, 0 compression engines reported" << std::endl;
        return;
    }

    infos_.resize( nCompressors );
    for( size_t i = 0; i < nCompressors; ++i )
    {
        CompressorInfo& info = infos_[ i ];
        info.version = EQ_COMPRESSOR_VERSION;
        info.outputTokenType = EQ_COMPRESSOR_DATATYPE_NONE;
        info.outputTokenSize = 0;
        getInfo( i, &info );

        if(( info.capabilities & EQ_COMPRESSOR_USE_ASYNC_DOWNLOAD ) &&
            ( !startDownload || !finishDownload ))
        {
            LBWARN << "Download plugin claims to support async readback " <<
                      "but corresponding functions are missing" << std::endl;
            infos_.clear();
            return;
        }
        info.ratingAlpha = powf( info.speed, .3f ) / info.ratio;
        info.ratingNoAlpha = info.ratingAlpha;

        if( info.capabilities & EQ_COMPRESSOR_IGNORE_ALPHA )
        {
            switch( info.tokenType )
            {
              case EQ_COMPRESSOR_DATATYPE_4_BYTE:
              case EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT:
              case EQ_COMPRESSOR_DATATYPE_4_FLOAT:
                  info.ratingNoAlpha /= .75f;
                  break;

              case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
                  info.ratingNoAlpha /= .9375f; // 30/32
                  break;

              default:
                  break;
            }
        }

        if( !( info.capabilities & EQ_COMPRESSOR_TRANSFER ))
        {
            if( info.outputTokenType == EQ_COMPRESSOR_DATATYPE_NONE )
            {
                // Set up CPU compressor output to be input type
                info.outputTokenType = info.tokenType;
                LBASSERT( info.outputTokenSize == 0 );
            }
            else
            {
                LBASSERT( info.outputTokenSize != 0 );
            }
        }
#ifndef NDEBUG // Check that each compressor exist once
        for( size_t j = 0; j < i; ++j )
        {
            LBASSERTINFO( info.name != infos_[j].name,
                          "Compressors " << i << " and " << j << " in '" <<
                          libraryName << "' use the same name" );
        }
#endif

    }

    LBINFO << "Loaded " << nCompressors << " plugins from " << libraryName
           << std::endl;
}

bool Plugin::isGood() const
{
    return !infos_.empty();
}

void Plugin::initChildren( const PluginRegistry& registry )
{
    const Plugins& plugins = registry.getPlugins();

    for( CompressorInfos::iterator i = infos_.begin(); i != infos_.end(); ++i )
    {
        CompressorInfo& info = *i;
        LBLOG( LOG_PLUGIN ) << lunchbox::disableFlush << "Engine 0x" << std::hex
                            << info.name;

        if( info.capabilities & EQ_COMPRESSOR_TRANSFER )
        {
            LBLOG( LOG_PLUGIN ) << " compressors:";
            // Find compressors for downloader
            for( Plugins::const_iterator j = plugins.begin();
                 j != plugins.end(); ++j )
            {
                const Plugin* plugin = *j;
                const CompressorInfos& infos = plugin->getInfos();

                for( CompressorInfos::const_iterator k = infos.begin();
                     k != infos.end(); ++k )
                {
                    const CompressorInfo& child = *k;
                    if( child.capabilities & EQ_COMPRESSOR_TRANSFER ||
                        child.tokenType != info.outputTokenType )
                    {
                        continue;
                    }

                    info.compressors.push_back( child );
                    LBLOG( LOG_PLUGIN ) << " 0x" << child.name;
                }
            }
        }
        else
        {
            LBLOG( LOG_PLUGIN ) << " uploaders:";
            // Find uploaders for decompressor
            for( Plugins::const_iterator j = plugins.begin();
                 j != plugins.end(); ++j )
            {
                const Plugin* plugin = *j;
                const CompressorInfos& infos = plugin->getInfos();

                for( CompressorInfos::const_iterator k = infos.begin();
                     k != infos.end(); ++k )
                {
                    const CompressorInfo& child = *k;
                    if( !(child.capabilities & EQ_COMPRESSOR_TRANSFER) ||
                        child.tokenType != info.outputTokenType )
                    {
                        continue;
                    }

                    info.uploaders.push_back( child );
                    LBLOG( LOG_PLUGIN ) << " 0x" << child.name;
                }
            }
        }
        LBLOG( LOG_PLUGIN ) << std::endl << std::dec << lunchbox::enableFlush;
    }
}

bool Plugin::implementsType( const uint32_t name ) const
{
    for( CompressorInfos::const_iterator i = infos_.begin();
         i != infos_.end(); ++i )
    {
        if ( i->name == name )
            return true;
    }

    return false;
}

const CompressorInfo& Plugin::findInfo( const uint32_t name ) const
{
    for( CompressorInfos::const_iterator i = infos_.begin();
         i != infos_.end(); ++i )
    {
        if( i->name == name )
            return (*i);
    }

    LBUNREACHABLE;
    return infos_.front();
}

}
