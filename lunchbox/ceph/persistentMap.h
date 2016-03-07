
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#ifdef LUNCHBOX_USE_RADOS
#include <lunchbox/compiler.h>
#include <lunchbox/log.h>

#include <rados/librados.hpp>

namespace lunchbox
{
namespace ceph
{
namespace
{
static void _throw( const std::string& reason, const int error )
{
    throw std::runtime_error( reason + ": " + ::strerror( -error ));
}
}

class PersistentMap : public detail::PersistentMap
{
public:
    PersistentMap( const servus::URI& uri )
    {
        const int init = _cluster.init2( uri.getUserinfo().c_str(),
                                         uri.getHost().c_str(),
                                         0 /*flags*/ );
        if( init < 0 )
            _throw( "Cannot initialize rados cluster", init );

        const int conf = _cluster.conf_read_file( uri.getPath().c_str( ));
        if( conf < 0 )
            _throw( "Cannot read ceph config '" + uri.getPath() + "'", conf );

        const int conn = _cluster.connect();
        if( conn < 0 )
            _throw( "Could not connect rados cluster", conn );
    }

    virtual ~PersistentMap()
    {
        _cluster.shutdown();
    }

    static bool handles( const servus::URI& uri )
        { return uri.getScheme() == "ceph"; }

    bool insert( const std::string& /*key*/, const void* /*data*/, const size_t /*size*/ )
        final
    {
        return false;
    }

    std::string operator [] ( const std::string& /*key*/ ) const final
    {
        return std::string();
    }

    bool contains( const std::string& /*key*/ ) const final
    {
        return false;
    }

    bool flush() final { /*NOP?*/ return true; }

private:
    librados::Rados _cluster;
};
}
}

#endif
