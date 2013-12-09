
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

#include "compressor.h"

#include "compressorInfo.h"
#include "plugin.h"
#include "pluginInstance.h"
#include "pluginRegistry.h"

#include <algorithm>

namespace lunchbox
{
namespace detail
{
class Compressor : public PluginInstance
{
public:
    Compressor() {}

    Compressor( lunchbox::PluginRegistry& registry, const uint32_t name )
    {
        setup( registry, name );
    }

    ~Compressor()
    {
        clear();
    }

    void clear()
    {
        if( instance )
            plugin->deleteCompressor( instance );
        PluginInstance::clear();
    }

    bool setup( lunchbox::PluginRegistry& registry, const uint32_t name )
    {
        if( instance && info.name == name )
            return true;

        clear();

        if( name <= EQ_COMPRESSOR_NONE )
            return true;

        plugin = registry.findPlugin( name );
        LBASSERT( plugin );
        if( !plugin )
            return false;

        instance = plugin->newCompressor( name );
        info = plugin->findInfo( name );
        LBASSERT( instance );
        LBASSERT( info.name == name );
        LBLOG( LOG_PLUGIN ) << "Instantiated compressor of type 0x" << std::hex
                            << name << std::dec << std::endl;
        return instance;
    }
};
}

Compressor::Compressor()
    : impl_( new detail::Compressor )
{
    LB_TS_THREAD( _thread );
}

Compressor::Compressor( PluginRegistry& registry, const uint32_t name )
    : impl_( new detail::Compressor( registry, name ))
{
    LB_TS_THREAD( _thread );
}

Compressor::~Compressor()
{
    delete impl_;
}

bool Compressor::isGood() const
{
    LB_TS_SCOPED( _thread );
    return impl_->isGood() && impl_->instance;
}

bool Compressor::uses( const uint32_t name ) const
{
    return isGood() && impl_->info.name == name;
}

const EqCompressorInfo& Compressor::getInfo() const
{
    return impl_->info;
}

uint32_t Compressor::choose( const PluginRegistry& registry,
                             const uint32_t tokenType, const float minQuality,
                             const bool ignoreAlpha )
{
    LBASSERT( tokenType != EQ_COMPRESSOR_DATATYPE_NONE );
    CompressorInfo candidate;
    candidate.name = EQ_COMPRESSOR_NONE;
    candidate.ratingAlpha = 1.0f;
    candidate.quality = 1.0f;
    candidate.speed = 1.0f;

    const Plugins& plugins = registry.getPlugins();
    for( Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i )
    {
        const Plugin* plugin = *i;
        const CompressorInfos& infos = plugin->getInfos();

        for( CompressorInfosCIter j = infos.begin(); j != infos.end(); ++j )
        {
            const CompressorInfo& info = *j;
            if( info.tokenType != tokenType || info.quality < minQuality ||
                ( info.capabilities & EQ_COMPRESSOR_TRANSFER ))
            {
                continue;
            }

            const float rating = ignoreAlpha ? info.ratingNoAlpha :
                                               info.ratingAlpha;
            if( rating > candidate.ratingAlpha )
            {
                candidate = info;
                candidate.ratingAlpha = rating;
            }
        }
    }

    return candidate.name;
}

bool Compressor::setup( PluginRegistry& registry, const uint32_t name )
{
    LB_TS_THREAD( _thread );
    return impl_->setup( registry, name );
}

bool Compressor::setup( PluginRegistry& registry, const uint32_t tokenType,
                        const float minQuality, const bool ignoreMSE )
{
    return impl_->setup( registry,
                         choose( registry, tokenType, minQuality, ignoreMSE ));
}

bool Compressor::realloc()
{
    if( !isGood( ))
        return false;

    impl_->plugin->deleteCompressor( impl_->instance );
    impl_->instance = impl_->plugin->newCompressor( impl_->info.name );

    LBASSERT( impl_->instance );
    return impl_->instance;
}

void Compressor::clear()
{
    impl_->clear();
}

void Compressor::compress( void* const in, const uint64_t pvpIn[4],
                           const uint64_t flags )
{
    LBASSERT( impl_->plugin );
    LBASSERT( impl_->instance );
    LBASSERT( in );
    if( !isGood( ))
        return;

    impl_->plugin->compress( impl_->instance, impl_->info.name, in, pvpIn,
                             flags );
}

void Compressor::compress( void* const in, const uint64_t inDims[2] )
{
    LBASSERT( impl_->plugin );
    LBASSERT( impl_->instance );
    LBASSERT( in );
    if( !isGood( ))
        return;

    impl_->plugin->compress( impl_->instance, impl_->info.name, in, inDims,
                             EQ_COMPRESSOR_DATA_1D );
}

unsigned Compressor::getNumResults() const
{
    LBASSERT( impl_->plugin );
    LBASSERT( impl_->instance );
    if( !isGood( ))
        return 0;

    return impl_->plugin->getNumResults( impl_->instance, impl_->info.name );
}

void Compressor::getResult( const unsigned i, void** const out,
                            uint64_t* const outSize ) const
{
    LBASSERT( impl_->plugin );
    LBASSERT( impl_->instance );
    if( !isGood( ))
        return;

    return impl_->plugin->getResult( impl_->instance, impl_->info.name, i, out,
                                     outSize );
}
}
