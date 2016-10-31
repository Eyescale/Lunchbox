
/* Copyright (c) 2014-2016, Stefan.Eilemann@epfl.ch
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

#ifdef LUNCHBOX_USE_LEVELDB
#include <lunchbox/compiler.h>
#include <lunchbox/log.h>

#include <leveldb/db.h>

namespace lunchbox
{
namespace db = ::leveldb;

namespace leveldb
{
namespace
{
db::DB* _open( const servus::URI& uri )
{
    db::DB* db = 0;
    db::Options options;
    options.create_if_missing = true;
    const std::string& path = uri.getPath().empty() ?
                              "persistentMap.leveldb" : uri.getPath();
    const db::Status status = db::DB::Open( options, path, &db );
    if( !status.ok( ))
        LBTHROW( std::runtime_error( status.ToString( )));
    return db;
}
}

class PersistentMap : public detail::PersistentMap
{
public:
    explicit PersistentMap( const servus::URI& uri ) : _db( _open( uri )) {}

    virtual ~PersistentMap() { delete _db; }

    static bool handles( const servus::URI& uri )
        { return uri.getScheme() == "leveldb" || uri.getScheme().empty(); }

    bool insert( const std::string& key, const void* data, const size_t size )
        final
    {
        const db::Slice value( (const char*)data, size );
        return _db->Put( db::WriteOptions(), key, value ).ok();
    }

    std::string operator [] ( const std::string& key ) const final
    {
        std::string value;
        if( _db->Get( db::ReadOptions(), key, &value ).ok( ))
            return value;
        return std::string();
    }

    void takeValues( const Strings& keys, const ValueFunc& func ) const final
    {
        for( const auto& key: keys )
        {
            std::string value;
            if( !_db->Get( db::ReadOptions(), key, &value ).ok( ))
                continue;

            char* copy = (char*)malloc( value.size( ));
            memcpy( copy, value.data(), value.size( ));
            func( key, copy, value.size( ));
        }
    }

    void getValues( const Strings& keys, const ConstValueFunc& func ) const
        final
    {
        for( const auto& key: keys )
        {
            std::string value;
            if( !_db->Get( db::ReadOptions(), key, &value ).ok( ))
                continue;

            func( key, value.data(), value.size( ));
        }
    }

    bool flush() final { /*NOP?*/ return true; }

private:
    db::DB* const _db;
};
}
}

#endif
