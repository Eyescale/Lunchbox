
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

#include "compressorInfo.h"
#include "os.h"

namespace
{
class PluginInstance
{
public:
    PluginInstance( const uint32_t name )
        : plugin( 0 )
        , instance( 0 )
    {
        lunchbox::setZero( &info, sizeof( info ));
        info.name = name;
    }

    ~PluginInstance()
    {
        LBASSERT( !plugin );
        LBASSERT( !instance );
    }

    bool isGood() const
    {
        return ( plugin && info.name != EQ_COMPRESSOR_INVALID &&
                 info.name != EQ_COMPRESSOR_NONE );
    }

    /** Plugin handling the allocation */
    lunchbox::Plugin* plugin;

    /** The instance of the (de)compressor, may be 0 for decompressor */
    void* instance;

    /** Info about the current compressor instance */
    EqCompressorInfo info;
};
}
