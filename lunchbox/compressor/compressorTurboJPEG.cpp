
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

#include "compressorTurboJPEG.h"

#include <iostream>
#include <math.h>

#define QUOTE( string ) STRINGIFY( string )
#define STRINGIFY( foo ) #foo

namespace lunchbox
{
namespace plugin
{
namespace
{
    static int _version; // Eq plugin API version
    static bool _init = tjInitCompress();
    typedef const char* ( *GetKey_t ) ();

#define REGISTER_ENGINE( token_, name_, quality_, ratio_, speed_, alpha )    \
    static void _getInfoTurbo ## token_ ## name_ ## alpha( EqCompressorInfo* const info ) \
    {                                                                        \
        _version = info->version;                                            \
        info->version = EQ_COMPRESSOR_VERSION;                               \
        info->capabilities = EQ_COMPRESSOR_DATA_2D;                          \
        if( alpha )                                                          \
            info->capabilities |= EQ_COMPRESSOR_IGNORE_ALPHA;                \
        info->quality = quality_ ## f;                                       \
        info->ratio   = ratio_ ## f;                                         \
        info->speed   = speed_ ## f;                                         \
        info->name = EQ_COMPRESSOR_CH_EYESCALE_JPEG_## token_ ## name_;      \
        info->tokenType = EQ_COMPRESSOR_DATATYPE_ ## token_;                 \
        if( alpha )                                                          \
        {                                                                    \
            if( _version > 2 )                                               \
            {                                                                \
                info->outputTokenType = EQ_COMPRESSOR_DATATYPE_ ## token_;   \
                info->outputTokenSize = 3;                                   \
            }                                                                \
        }                                                                    \
        else if( _version < 3 )                                              \
            info->tokenType = EQ_COMPRESSOR_DATATYPE_INVALID;                \
    }                                                                        \
                                                                             \
    static bool _registerTurbo ## token_ ## name_ ## alpha()                 \
    {                                                                        \
        Compressor::registerEngine(                                          \
            Compressor::Functions(                                           \
                EQ_COMPRESSOR_CH_EYESCALE_JPEG_ ## token_ ## name_,          \
               _getInfoTurbo ## token_ ## name_ ## alpha,                    \
               CompressorTurboJPEG::getNewCompressor,                        \
               CompressorTurboJPEG::getNewCompressor,                        \
               CompressorTurboJPEG::decompress, 0 ));                        \
        return true;                                                         \
    }                                                                        \
                                                                             \
    static bool _initialized ## token_ ## name_ ## alpha =                   \
        _registerTurbo ## token_ ## name_ ## alpha();

REGISTER_ENGINE( RGBA, 100, 0.9, 0.33, 0.34, true );
REGISTER_ENGINE( BGRA, 100, 0.9, 0.33, 0.34, true );

REGISTER_ENGINE( RGBA, 90, 0.8, 0.09, 0.65, true );
REGISTER_ENGINE( BGRA, 90, 0.8, 0.09, 0.65, true );

REGISTER_ENGINE( RGBA, 80, 0.7, 0.07, 0.75, true );
REGISTER_ENGINE( BGRA, 80, 0.7, 0.07, 0.75, true );

REGISTER_ENGINE( RGB, 100, 0.9, 0.3, 1.2, false );
REGISTER_ENGINE( RGB, 90, 0.8, 0.3, 1.2, false );
REGISTER_ENGINE( RGB, 80, 0.7, 0.3, 1.2, false );
REGISTER_ENGINE( BGR, 100, 0.9, 0.3, 1.2, false );
REGISTER_ENGINE( BGR, 90, 0.8, 0.3, 1.2, false );
REGISTER_ENGINE( BGR, 80, 0.7, 0.3, 1.2, false );
}

CompressorTurboJPEG::CompressorTurboJPEG( const unsigned name )
     : Compressor()
     , _quality( 100 )
     , _tokenSize( 4 )
     , _flags( 0 )
     , _encoder( 0 )
     , _decoder( 0 )
{
    switch( name )
    {
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGRA80:
            _flags = TJ_BGR;
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGBA80:
            _quality = 80;
            break;

        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGRA90:
            _flags = TJ_BGR;
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGBA90:
            _quality = 90;
            break;

        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGRA100:
            _flags = TJ_BGR;
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGBA100:
            break;

        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGR80:
            _flags = TJ_BGR;
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGB80:
            _quality = 80;
            _tokenSize = 3;
            break;

        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGR90:
            _flags = TJ_BGR;
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGB90:
            _quality = 90;
            _tokenSize = 3;
            break;

        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGR100:
            _flags = TJ_BGR;
        case EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGB100:
            _tokenSize = 3;
            break;

        default:
            assert( false );
    }

    _flags = _flags | TJ_FASTUPSAMPLE;
    _results.push_back( new Result ); // RGB jpeg
    _results.push_back( new Result ); // Alpha uncompressed TODO RLE
}

CompressorTurboJPEG::~CompressorTurboJPEG()
{
    if( _decoder )
        tjDestroy( _decoder );
    _decoder = 0;

    if( _encoder )
        tjDestroy( _encoder );
    _encoder = 0;

}

void CompressorTurboJPEG::compress( const void* const inData,
                                    const eq_uint64_t* inDims,
                                    const eq_uint64_t flags )
{
    assert( !_decoder );
    assert( flags & EQ_COMPRESSOR_DATA_2D );

    if( !_encoder )
        _encoder = tjInitCompress();

    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_ALPHA);
    if( useAlpha && _tokenSize == 4 )
    {
        const eq_uint64_t size = inDims[3] * inDims[1];
        _extractAlpha( reinterpret_cast< const unsigned char* >(inData), size );
        _nResults = 2;
    }
    else
        _nResults = 1;

    _results[0]->resize( TJBUFSIZE( inDims[1], inDims[3] ) );
    unsigned long size = 0;

    void* const data = const_cast< void* const  >(inData);
    if( tjCompress( _encoder, reinterpret_cast< unsigned char*>( data ),
                    inDims[1], inDims[1] * _tokenSize, inDims[3],
                    _tokenSize, _results[0]->getData(), &size,
                    TJ_444, _quality, _flags ))
    {
        assert( false );
        size = 0;
    }
    _results[0]->resize( size );
}

void CompressorTurboJPEG::decompress( const void* const* inData,
                                      const eq_uint64_t* const inSizes,
                                      const unsigned nInputs,
                                      void* const outData,
                                      eq_uint64_t* const outDims,
                                      const eq_uint64_t flags,
                                      void* const instance )
{
    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_ALPHA);
    static_cast< CompressorTurboJPEG* >( instance )->
        _decompress( inData, inSizes[0], nInputs, outData, outDims, useAlpha );
}

void CompressorTurboJPEG::_decompress( const void* const* inData,
                                       const eq_uint64_t inSize,
                                       const unsigned nInputs,
                                       void* const outData,
                                       eq_uint64_t* const outDims,
                                       const bool useAlpha )
{
    assert( !_encoder );
    if( !_decoder )
        _decoder = tjInitDecompress();
    void* const data = const_cast< void* const >( inData[0] );

    if( tjDecompress( _decoder, reinterpret_cast< unsigned char* >(data),
                      inSize, reinterpret_cast< unsigned char*>(outData),
                      outDims[1], outDims[1] * _tokenSize, outDims[3],
                      _tokenSize, _flags ))
    {
        assert( false );
    }
    else if( useAlpha && _tokenSize == 4 )
    {
        assert( nInputs == 2 );
        const eq_uint64_t size = outDims[3] * outDims[1];
        _addAlpha( inData[1], reinterpret_cast< unsigned* >( outData ), size);
    }

}

void CompressorTurboJPEG::_extractAlpha( const unsigned char* inData,
                                         const eq_uint64_t nPixels )
{
    _results[1]->resize( nPixels );

    const unsigned char* end = inData + nPixels * 4;
    unsigned char* dst = _results[1]->getData();
    for( const unsigned char* src = ( inData + 3 ); src < end; src += 4 )
    {
        *dst = *src;
        ++dst;
    }
}

void CompressorTurboJPEG::_addAlpha( const void* const in, unsigned* out,
                                     const eq_uint64_t nPixels ) const
{
    assert( _tokenSize == 4 );

    const unsigned char* alpha = reinterpret_cast<const unsigned char* >( in );
    const unsigned* end = out + nPixels;
    for(unsigned* i = out; i < end; ++i, ++alpha )
        *i = ((*i) & 0xffffffu) + ((*alpha)<<24);
}

}
}
