
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
#include <lunchbox/log.h>
#include <lunchbox/stdExt.h>

#include <rados/librados.hpp>
#include <boost/scoped_ptr.hpp>

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
        : _maxPendingOps( 0 )
    {
        const int init = _cluster.init2( uri.getUserinfo().c_str(),
                                         "ceph",
                                         0 /*flags*/ );
        if( init < 0 )
            _throw( "Cannot initialize rados cluster", init );

        const int conf = _cluster.conf_read_file( uri.getPath().c_str( ));
        if( conf < 0 )
            _throw( "Cannot read ceph config '" + uri.getPath() + "'", conf );

        const int conn = _cluster.connect();
        if( conn < 0 )
            _throw( "Could not connect rados cluster", conn );

        const int ctx = _cluster.ioctx_create( uri.getHost().c_str(),
                                               _context );
        if( ctx < 0 )
            _throw( "Could not create io context", ctx );

    }

    virtual ~PersistentMap()
    {
        _context.close();
        _cluster.shutdown();
    }

    static bool handles( const servus::URI& uri )
        { return uri.getScheme() == "ceph"; }

    size_t setQueueDepth( const size_t depth ) final
    {
        _maxPendingOps = depth;
        //LBCHECK( _flush( _maxPendingOps ));
        return _maxPendingOps;
    }

    bool insert( const std::string& key, const void* data, const size_t size )
        final
    {
        librados::bufferlist bl;
        bl.append( (const char*)data, size );

        if( _maxPendingOps == 0 ) // sync write
        {
            const int write = _context.write_full( key, bl );
            if( write >= 0 )
                return true;

            std::cerr <<  "Write failed: " << ::strerror( -write ) << std::endl;
            return false;
        }

        librados::AioCompletion* op = librados::Rados::aio_create_completion();
        const int write = _context.aio_write_full( key, op, bl );
        if( write < 0 )
        {
            std::cerr <<  "Write failed: " << ::strerror( -write )
                      << std::endl;
            delete op;
            return false;
        }
        _writes.push_back( op );
        return _flush( _maxPendingOps );
    }

    bool fetch( const std::string& key, const size_t sizeHint ) const final
    {
        if( _reads.size() >= _maxPendingOps )
            return true;

        AsyncRead& asyncRead = _reads[ key ];
        if( asyncRead.op )
            return true; // fetch for key already pending

        asyncRead.op.reset( librados::Rados::aio_create_completion( ));
        uint64_t size = sizeHint;
        if( size == 0 )
        {
            time_t time;
            const int stat = _context.stat( key, &size, &time );
            if( stat < 0 || size == 0 )
            {
                std::cerr << "Stat " << key << " failed: "
                          << ::strerror( -stat ) << std::endl;
                return false;
            }
        }

        const int read = _context.aio_read( key, asyncRead.op.get(),
                                            &asyncRead.bl, size, 0 );
        if( read >= 0 )
            return true;

        std::cerr <<  "Fetch failed: " << ::strerror( -read ) << std::endl;
        return false;
    }

    std::string operator [] ( const std::string& key ) const final
    {
        ReadMap::iterator i = _reads.find( key );
        librados::bufferlist bl;
        if( i == _reads.end( ))
        {
            uint64_t size = 0;
            time_t time;
            const int stat = _context.stat( key, &size, &time );
            if( stat < 0 || size == 0 )
            {
                std::cerr << "Stat '" << key << "' failed: "
                          << ::strerror( -stat ) << std::endl;
                return std::string();
            }

            const int read = _context.read( key, bl, size, 0 );
            if( read < 0 )
            {
                std::cerr << "Read '" << key << "' failed: "
                          << ::strerror( -read ) << std::endl;
                return std::string();
            }
        }
        else
        {
            i->second.op->wait_for_complete();
            const int read = i->second.op->get_return_value();
            if( read < 0 )
            {
                std::cerr <<  "Finish read '" << key << "' failed: "
                          << ::strerror( -read ) << std::endl;
                return std::string();
            }

            i->second.bl.swap( bl );
            _reads.erase( i );
        }
        std::string value;
        bl.copy( 0, bl.length(), value );
        return value;
    }

    bool contains( const std::string& key ) const final
    {
        uint64_t size;
        time_t time;
        return _context.stat( key, &size, &time ) >= 0;
    }

    bool flush() final { return _flush( 0 ); }

private:
    librados::Rados _cluster;
    mutable librados::IoCtx _context;
    size_t _maxPendingOps;

    typedef boost::scoped_ptr< librados::AioCompletion > AioPtr;

    struct AsyncRead
    {
        AsyncRead() {}
        AioPtr op;
        librados::bufferlist bl;
    };

    typedef stde::hash_map< std::string, AsyncRead > ReadMap;
    typedef std::deque< librados::AioCompletion* > Writes;

    mutable ReadMap _reads;
    Writes _writes;

    bool _flush( const size_t maxPending )
    {
        if( maxPending == 0 )
        {
            const int flushAll = _context.aio_flush();
            while( !_writes.empty( ))
            {
                delete _writes.front();
                _writes.pop_front();
            }
            if( flushAll >= 0 )
                return true;

            std::cerr <<  "Flush all writes failed: " << ::strerror( -flushAll )
                      << std::endl;
            return false;
        }

        bool ok = true;
        while( _writes.size() > maxPending )
        {
            _writes.front()->wait_for_complete();
            const int write = _writes.front()->get_return_value();
            if( write < 0 )
            {
                std::cerr <<  "Finish write failed: " << ::strerror( -write )
                          << std::endl;
                ok = false;
            }
            delete _writes.front();
            _writes.pop_front();
        }
        return ok;
    }
};
}
}

#endif
