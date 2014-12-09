
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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
#include <lunchbox/readyFuture.h>

#include <leveldb/db.h>

namespace lunchbox
{
namespace db = ::leveldb;

namespace leveldb
{
namespace
{

db::DB* _open( const URI& uri )
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
    PersistentMap( const URI& uri ) : _db( _open( uri )) {}

    virtual ~PersistentMap() { delete _db; }

    static bool handles( const URI& uri )
        { return uri.getScheme() == "leveldb"; }

    f_bool_t insert( const std::string& key, const void* data, const size_t size )
        final
    {
        const db::Slice value( (const char*)data, size );
        if( _db->Put( db::WriteOptions(), key, value ).ok( ))
            return makeTrueFuture();
        return makeFalseFuture();
    }

    std::string operator [] ( const std::string& key ) const final
    {
        std::string value;
        if( _db->Get( db::ReadOptions(), key, &value ).ok( ))
            return value;
        return std::string();
    }

    bool contains( const std::string& key ) const final
    {
        std::string value;
        return _db->Get( db::ReadOptions(), key, &value ).ok();
    }

private:
    db::DB* const _db;
};
}
}

#endif
