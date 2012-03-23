
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include <lunchbox/version.h>
#include <sstream>

#define QUOTE( string ) STRINGIFY( string )
#define STRINGIFY( foo ) #foo

namespace lunchbox
{

uint32_t Version::getMajor() 
{
    return LUNCHBOX_VERSION_MAJOR; 
}
uint32_t Version::getMinor()
{
    return LUNCHBOX_VERSION_MINOR; 
}
uint32_t Version::getPatch() 
{
    return LUNCHBOX_VERSION_PATCH; 
}
std::string Version::getRevision() 
{
    return std::string( QUOTE( LUNCHBOX_VERSION_REVISION ));
}
uint32_t Version::getABI() 
{
    return LUNCHBOX_VERSION_ABI; 
}

uint32_t Version::getInt()
{
    return ( LUNCHBOX_VERSION_MAJOR * 10000 +
             LUNCHBOX_VERSION_MINOR * 100   +
             LUNCHBOX_VERSION_PATCH ); 
}
float Version::getFloat() 
{
    return ( LUNCHBOX_VERSION_MAJOR +
             .01f   * LUNCHBOX_VERSION_MINOR   +
             .0001f * LUNCHBOX_VERSION_PATCH ); 
}
std::string Version::getString()
{
    std::ostringstream  version;
    version << LUNCHBOX_VERSION_MAJOR << '.' << LUNCHBOX_VERSION_MINOR;
    if( LUNCHBOX_VERSION_PATCH > 0 )
        version << '.' << LUNCHBOX_VERSION_PATCH;

    const std::string revision = getRevision();
    if( revision != "0" )
        version << '.' << revision;

    return version.str();
}

}
