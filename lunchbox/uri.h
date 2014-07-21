
/* Copyright (c) 2013-2014, ahmet.bilgili@epfl.ch
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

#ifndef URI_H
#define URI_H

#include <lunchbox/types.h>

namespace lunchbox
{
namespace detail { class URI; }

/**
 * The URI class parses the given uri string according to the regex given in
 * RFC3986.
 * http://bob@www.example.com:8080/path/?key=value#fragment
 * ^   ^  ^  ^               ^    ^     ^         ^
 * a   b  c  d               e    f     g         h
 *
 * URI part	Range	String
 * scheme	[a, b)	"http"
 * userinfo [c, d) bob
 * host	[d, e)	"www.example.com"
 * port (e, f) 8080
 * path	[f, g)	"/path/"
 * query [g, h)	"?key=value"
 * fragment	(h,-) "fragment"
 */
class URI
{
public:

    /**
     * @param uri URI string to parse.
     * @throws Throws std::exception for incomplete URIs, and throws
     * boost::bad_lexical_cast if port is not a number.
     * @version 1.9.2
     */
    URI( const std::string& uri );

    ~URI();

    /** @name URIGetters Getter methods for the uri.
     *  @version 1.9.2
     */
    ///@{
    const std::string& getScheme() const;
    const std::string& getUserinfo() const;
    uint16_t getPort() const;
    const std::string& getHost() const;
    const std::string& getPath() const;
    const std::string& getQuery() const;
    const std::string& getFragment() const;
     ///@}

private:

    detail::URI const *_impl;
};

inline std::ostream& operator<<( std::ostream& os, const URI& uri )
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
#endif // URI_H
