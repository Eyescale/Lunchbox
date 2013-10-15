
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

#define TEST_RUNTIME 600 // seconds
#include <test.h>

#include <lunchbox/buffer.h>
#include <lunchbox/clock.h>
#include <lunchbox/compressor.h>
#include <lunchbox/compressorInfo.h>
#include <lunchbox/decompressor.h>
#include <lunchbox/file.h>
#include <lunchbox/memoryMap.h>
#include <lunchbox/plugin.h>
#include <lunchbox/pluginRegistry.h>
#include <lunchbox/rng.h>

#include <algorithm>

using namespace lunchbox;

void _testFile();
void _testRandom();
void _testData( const uint32_t nameCompressor, const std::string& name,
                const uint8_t* data, const uint64_t size );

std::vector< uint32_t > getCompressorNames( const uint32_t tokenType );
std::vector< std::string > getFiles( const std::string path,
                                     std::vector< std::string >& files,
                                     const std::string& ext );

PluginRegistry registry;
uint64_t _result = 0;
uint64_t _size = 0;
float _compressionTime = 0;
float _decompressionTime = 0;
float _baseTime = 0.f;

int main( int, char** )
{
    registry.addDirectory( std::string( LUNCHBOX_BUILD_DIR ) + "/lib" );
    TEST( registry.addLunchboxPlugins( ));
    registry.init();

    _testFile();
    _testRandom();
    registry.exit();
    return EXIT_SUCCESS;
}

std::vector< uint32_t > getCompressorNames( const uint32_t tokenType )
{
    const Plugins& plugins = registry.getPlugins();
    std::vector< uint32_t > names;

    for( PluginsCIter i = plugins.begin(); i != plugins.end(); ++i )
    {
        const CompressorInfos& infos = (*i)->getInfos();
        for( CompressorInfosCIter j = infos.begin(); j != infos.end(); ++j )
        {
            if ( (*j).tokenType == tokenType )
                names.push_back( (*j).name );
        }
    }

    std::sort( names.begin(), names.end( ));
    return names;
}

void _testData( const uint32_t compressorName, const std::string& name,
                const uint8_t* data, const uint64_t size )
{
    Compressor compressor( registry, compressorName );
    Decompressor decompressor( registry, compressorName );
    TEST( compressor.isGood( ));
    TESTINFO( decompressor.isGood(), compressorName );

    const uint64_t flags = EQ_COMPRESSOR_DATA_1D;
    uint64_t inDims[2]  = { 0, size };

    compressor.compress( const_cast<uint8_t*>(data), inDims, flags );
    Clock clock;
    compressor.compress( const_cast<uint8_t*>(data), inDims, flags );
    const float compressTime = clock.getTimef();

    const unsigned numResults = compressor.getNumResults( );

    std::vector< void * > vectorVoid;
    vectorVoid.resize( numResults );

    std::vector< uint64_t > vectorSize;
    vectorSize.resize(numResults);

    uint64_t compressedSize = 0;
    for( unsigned i = 0; i < numResults ; i++ )
    {
        compressor.getResult( i, &vectorVoid[i], &vectorSize[i] );
        compressedSize += vectorSize[i];
    }

    Bufferb result;
    result.resize( size );
    uint8_t* outData = result.getData();

    decompressor.decompress( &vectorVoid.front(), &vectorSize.front(),
                             numResults, outData, inDims );

    clock.reset();
    decompressor.decompress( &vectorVoid.front(), &vectorSize.front(),
                             numResults, outData, inDims );
    const float decompressTime = clock.getTimef();

    TEST( memcmp( outData, data, size ) == 0 );
    std::cout  << std::setw(20) << name << ", 0x" << std::setw(8)
               << std::setfill( '0' ) << std::hex << compressorName << std::dec
               << std::setfill(' ') << ", " << std::setw(10) << size << ", "
               << std::setw(10) << compressedSize << ", " << std::setw(10)
               << compressTime << ", " << std::setw(10) << decompressTime
               << std::endl;
    _size += size;
    _result += compressedSize;
    _compressionTime += compressTime;
    _decompressionTime += decompressTime;
}

void _testFile()
{
    std::vector< uint32_t >compressorNames =
        getCompressorNames( EQ_COMPRESSOR_DATATYPE_BYTE );

    std::vector< std::string > files;

    getFiles( "", files, ".*\\.dll" );
    getFiles( "", files, ".*\\.exe" );
    getFiles( "", files, ".*\\.so" );
    getFiles( "../bin", files, ".*\\.dll" );
    getFiles( "../lib", files, ".*\\.so" );
    getFiles( "../../install/bin", files, ".*\\.dll" );
    getFiles( "../../install/lib", files, ".*\\.so" );
    getFiles( "images", files, ".*\\.rgb" );
    getFiles( "", files, ".*\\.a" );
    getFiles( "", files, ".*\\.dylib" );
    getFiles( "/Users/eile/Library/Models/mediumPly/", files, ".*\\.bin" );
    getFiles( "/Users/eile/Library/Models/mediumPly/", files, ".*\\.ply" );
    getFiles( "/home/eilemann/Software/Models/mediumPly/", files, ".*\\.bin" );
    getFiles( "/home/eilemann/Software/Models/mediumPly/", files, ".*\\.ply" );

    // Limit to 30 files using a pseudo-random selection for reproducability
    const size_t maxFiles = 30;
    if( files.size() > maxFiles )
    {
        const size_t cut = files.size() - maxFiles;
        for( size_t i = 0; i < cut; ++i )
            files.erase( files.begin() + (i * 997 /*prime*/) % files.size( ));
    }

    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "                File, Compressor, Uncompress, "
              << "Compressed,     t_comp,   t_decomp" << std::endl;
    for( std::vector< uint32_t >::const_iterator i = compressorNames.begin();
         i != compressorNames.end(); ++i )
    {
        _result = 0;
        _size = 0;
        _compressionTime = 0;
        _decompressionTime = 0;

        for( StringsCIter j = files.begin(); j != files.end(); ++j )
        {
            MemoryMap file;
            const uint8_t* data = static_cast<const uint8_t*>( file.map( *j ));

            if( !data )
            {
                LBERROR << "Can't mmap " << *j << std::endl;
                continue;
            }

            const size_t size = file.getSize();
            const std::string name = getFilename( *j );

            _testData( *i, name, data, size );
        }
        if( _baseTime == 0.f )
            _baseTime = _compressionTime + _decompressionTime;

        std::cout << std::setw(24) << "Total, 0x" << std::setw(8)
                  << std::setfill( '0' ) << std::hex << *i << std::dec
                  << std::setfill(' ') << ", " << std::setw(10) << _size << ", "
                  << std::setw(10) << _result << ", " << std::setw(10)
                  << _compressionTime << ", " << std::setw(10)
                  << _decompressionTime << std::endl
                  << "    info->ratio = " << float(_result) / float(_size)
                  << "f;" << std::endl
                  << "    info->speed = " << float(_baseTime) /
                                    float(_compressionTime + _decompressionTime)
                  << "f;" << std::endl << std::endl;
    }
}

void _testRandom()
{
    ssize_t size = LB_10MB;
    uint8_t* data = new uint8_t[size];
    RNG rng;
#pragma omp parallel for
    for( ssize_t i = 0; i < size; ++i )
        data[i] = rng.get< uint8_t >();

    std::vector< uint32_t >compressorNames =
        getCompressorNames( EQ_COMPRESSOR_DATATYPE_BYTE );
    _result = 0;
    _size = 0;
    _compressionTime = 0;
    _decompressionTime = 0;

    for( std::vector<uint32_t>::const_iterator i = compressorNames.begin();
         i != compressorNames.end(); ++i )
    {
        size = LB_10MB;
        for( size_t j = 0; j<8; ++j ) // test all granularities between mod 8..1
        {
            _testData( *i, "Random data", data, size );
            --size;
        }
        std::cout << std::setw(24) << "Total, 0x" << std::setw(8)
                  << std::setfill( '0' ) << std::hex << *i << std::dec
                  << std::setfill(' ') << ", " << std::setw(10) << _size << ", "
                  << std::setw(10) << _result << ", " << std::setw(10)
                  << _compressionTime << ", " << std::setw(10)
                  << _decompressionTime << std::endl << std::endl;
   }

    delete [] data;
}

std::vector< std::string > getFiles( const std::string path,
                                     std::vector< std::string >& files,
                                     const std::string& ext )
{
    Strings paths = registry.getDirectories();
    if( !path.empty( ))
        paths.push_back( path );

    for( uint64_t j = 0; j < paths.size(); ++j )
    {
        const Strings& candidates = searchDirectory( paths[j], ext.c_str( ));
        for( StringsCIter i = candidates.begin(); i != candidates.end(); ++i )
        {
            const std::string& filename = *i;
            files.push_back( paths[j] + '/' + filename );
        }
    }
    return files;
}
