
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

#include "decompressor.h"

#include "plugin.h"
#include "pluginInstance.h"
#include "pluginRegistry.h"

namespace lunchbox
{
namespace detail
{
class Decompressor : public PluginInstance
{
public:
    Decompressor( const uint32_t name ) : PluginInstance( name ) {}
};
}

Decompressor::Decompressor()
    : impl_( new detail::Decompressor( EQ_COMPRESSOR_NONE ))
{}

Decompressor::Decompressor( PluginRegistry& registry, const uint32_t name )
    : impl_( new detail::Decompressor( name ))
{
    LB_TS_THREAD( _thread );
    if( name <= EQ_COMPRESSOR_NONE )
        return;

    impl_->plugin = registry.findPlugin( name );
    LBASSERTINFO( impl_->plugin,
                  "Can't find plugin for decompressor " << name );
    if( !impl_->plugin )
        return;

    impl_->instance = impl_->plugin->newDecompressor( name );
    impl_->info = impl_->plugin->findInfo( name );
    LBASSERT( impl_->info.name == name );

    LBLOG( LOG_PLUGIN ) << "Instantiated " << (impl_->instance ? "" : "empty ")
                        << "decompressor of type 0x" << std::hex << name
                        << std::dec << std::endl;
}

Decompressor::~Decompressor()
{
    if( impl_->instance )
        impl_->plugin->deleteDecompressor( impl_->instance );
    impl_->clear();
    delete impl_;
}

bool Decompressor::isGood() const
{
    LB_TS_SCOPED( _thread );
    return impl_->isGood();
}

bool Decompressor::uses( const uint32_t name ) const
{
    return isGood() && impl_->info.name == name;
}

const Decompressor& Decompressor::operator = ( Decompressor& from )
{
    if( impl_->instance )
        impl_->plugin->deleteDecompressor( impl_->instance );
    *impl_ = *from.impl_;
    from.impl_->clear();
    return *this;
}

const EqCompressorInfo& Decompressor::getInfo() const
{
    return impl_->info;
}

void Decompressor::decompress( const void* const* in,
                               const uint64_t* const inSizes,
                               const unsigned numInputs, void* const out,
                               uint64_t pvpOut[4], const uint64_t flags )
{
    impl_->plugin->decompress( impl_->instance, impl_->info.name, in, inSizes,
                               numInputs, out, pvpOut, flags );
}

void Decompressor::decompress( const void* const* in,
                               const uint64_t* const inSizes,
                               const unsigned numInputs, void* const out,
                               uint64_t outDim[2] )
{
    impl_->plugin->decompress( impl_->instance, impl_->info.name, in, inSizes,
                               numInputs, out, outDim, EQ_COMPRESSOR_DATA_1D );
}
}
