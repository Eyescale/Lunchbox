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
 * @verbatim
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
 * @endverbatim
 *
 * Queries are parsed into key-value pairs and can be accessed using
 * findQuery(), queryBegin() and queryEnd().
 *
 * Example: @include tests/uri.cpp
 */
class URI
{
public:
    typedef boost::unordered_map< std::string, std::string > KVMap;
    typedef KVMap::const_iterator ConstKVIter;

    /**
     * @param uri URI string to parse.
     * @throw std::exception for incomplete URIs, and boost::bad_lexical_cast
     *        if the port is not a number.
     * @version 1.9.2
     */
    LUNCHBOX_API URI( const std::string& uri );

    /** Copy-construct an URI. @version 1.9.2 */
    LUNCHBOX_API URI( const URI& from );

    LUNCHBOX_API ~URI();

    /** Assign the data from another URI. @version 1.9.2 */
    LUNCHBOX_API URI& operator = ( const URI& rhs );

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

    /** @name Access to key-value data in query @version 1.9.2 */
    //@{
    /**
     * @return a const iterator to the beginning of the query map.
     * @version 1.9.2
     */
    LUNCHBOX_API ConstKVIter queryBegin() const;

    /**
     * @return a const iterator to end beginning of the query map.
     * @version 1.9.2
     */
    LUNCHBOX_API ConstKVIter queryEnd() const;

    /**
     * @return a const iterator to the given key, or queryEnd().
     * @version 1.9.2
     */
    LUNCHBOX_API ConstKVIter findQuery( const std::string& key ) const;

    /** Add a key-value pair to the query. @version 1.9.2 */
    LUNCHBOX_API void addQuery( const std::string& key,
                                const std::string& value );
    //@}

private:
    detail::URI* const _impl;
};

inline std::ostream& operator << ( std::ostream& os, const URI& uri )
{
    if( !uri.getScheme().empty( ))
        os << uri.getScheme() << "://";
    if( !uri.getUserinfo().empty( ))
        os << uri.getUserinfo() << "@";
    os << uri.getHost();
    if( uri.getPort( ))
        os << ':' << uri.getPort();
    os << uri.getPath();
    if( !uri.getQuery().empty( ))
        os << '?' << uri.getQuery();
    if( !uri.getFragment().empty( ))
        os << '#' << uri.getFragment();
    return os;
}
}

#  ifdef BOOST_LEXICAL_CAST_INCLUDED
namespace boost
{
template<> inline std::string lexical_cast( const lunchbox::URI& uri )
{
    std::ostringstream os;
    os << uri;
    return os.str();
}
}

#  endif
#endif // LUNCHBOX_URI_H
