
/* Copyright (c) 2013-2014, Stefan.Eilemann@epfl.ch
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
 * http://www.example.com/path/?key=value#fragment
 * ^   ^  ^              ^     ^         ^^       ^
 * a   b  c              d     e         fg       h
 *
 * URI part	Range	String
 * scheme	[a, b)	"http"
 * host	[c, d)	"www.example.com"
 * path	[d, e)	"/path/"
 * query	[e, f)	"?key=value"
 * fragment	[g, h)	"fragment"
 */
class URI
{
public:

    /**
     * @param uri URI string to parse.
     * @version 1.9.x
     */
    URI( const std::string& uri );

    ~URI();

    /** @name URIGetters Getter methods for the uri.
     *  @version 1.9.x
     */
    ///@{
    const std::string& getScheme() const;
    const std::string& getHost() const;
    const std::string& getPath() const;
    const std::string& getQuery() const;
    const std::string& getFragment() const;
     ///@}

    /**
     * @return True if URI string is empty.
     * @version 1.9.x
     */
    bool isEmpty() const;

    /**
     * @return Returns true if parsing is successful.
     * @version 1.9.x
     */
    bool isValid() const;

private:

    detail::URI const *_impl;
};

inline std::ostream& operator<<( std::ostream& os, const URI& uri )
{
    if( !uri.isValid( ))
        return os << "Invalid URI";

     os << uri.getScheme() << "://";
     os << uri.getHost();
     os << uri.getPath() << '?' << uri.getQuery();
     if( !uri.getFragment().empty( ))
         os << '#' << uri.getFragment();
     return os;
}

}
#endif // URI_H
