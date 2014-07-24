
/* Copyright (c) 2013-2014, ahmet.bilgili@epfl.ch
 *                    2014, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_URI_H
#define LUNCHBOX_URI_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>
#include <boost/unordered_map.hpp> // iterator typedefs

namespace lunchbox
{
namespace detail { class URI; }

/**
 * The URI class parses the given uri string according to the regex given in
 * RFC3986.
 * http://bob@www.example.com:8080/path/?key=value,foo=bar#fragment
 * ^   ^  ^  ^               ^    ^     ^                 ^
 * a   b  c  d               e    f     g                 h
 *
 * URI part	Range	String
 * scheme	[a, b)	"http"
 * userinfo [c, d) bob
 * host	[d, e)	"www.example.com"
 * port (e, f) 8080
 * path	[f, g)	"/path/"
 * query (g, h)	"key=value"
 * fragment	(h,-) "fragment"
 *
 * Queries are parsed into key-value pairs and can be accessed using TBD.
 *
 * Example: @include tests/uri.cpp
 */
class URI : public boost::noncopyable
{
public:
    typedef boost::unordered_map< std::string, std::string > KVMap;
    typedef KVMap::const_iterator ConstKVIter;

    /**
     * @param uri URI string to parse.
     * @throws Throws std::exception for incomplete URIs, and throws
     * boost::bad_lexical_cast if port is not a number.
     * @version 1.9.2
     */
    URI( const std::string& uri );
    ~URI();

    /** @name Getters for the uri data @version 1.9.2 */
    //@{
    LUNCHBOX_API const std::string& getScheme() const;
    LUNCHBOX_API const std::string& getUserinfo() const;
    LUNCHBOX_API uint16_t getPort() const;
    LUNCHBOX_API const std::string& getHost() const;
    LUNCHBOX_API const std::string& getPath() const;
    LUNCHBOX_API const std::string& getQuery() const;
    LUNCHBOX_API const std::string& getFragment() const;
    //@}

    /** @name Getters to query key-value data @version 1.9.2 */
    //@{
    LUNCHBOX_API ConstKVIter queryBegin() const;
    LUNCHBOX_API ConstKVIter queryEnd() const;
    LUNCHBOX_API ConstKVIter findQuery( const std::string& key ) const;
    //@}

private:
    detail::URI const *_impl;
};

inline std::ostream& operator << ( std::ostream& os, const URI& uri )
{
     os << uri.getScheme() << "://";
     if( !uri.getUserinfo().empty( ))
         os << uri.getUserinfo() << "@";
     os << uri.getHost();
     if( uri.getPort( ))
         os << ':' << uri.getPort();
     os << uri.getPath() << '?' << uri.getQuery();
     if( !uri.getFragment().empty( ))
         os << '#' << uri.getFragment();
     return os;
}

}
#endif // LUNCHBOX_URI_H
