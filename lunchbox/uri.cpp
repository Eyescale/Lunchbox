
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

#include <boost/regex.hpp>

#include <lunchbox/uri.h>

namespace lunchbox
{

namespace detail
{

struct URIData
{
    URIData() { clear(); }

    void clear()
    {
        uri.clear();
        scheme.clear();
        host.clear();
        path.clear();
        query.clear();
        fragment.clear();
    }

    std::string uri;
    std::string scheme;
    std::string host;
    std::string path;
    std::string query;
    std::string fragment;
};

class URIParser
{
public:

    URIParser(  ) { }

    bool parse( const std::string& uri, URIData& uriData )
    {

        boost::regex expr(
            "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?",
            boost::regex::perl|boost::regex::icase );

        boost::match_results< std::string::const_iterator > results;
        if( !boost::regex_search( uri, results, expr ) )
            return false;

        const std::string schema = std::string( results[2].first, results[2].second );
        if( schema.empty() )
            return false;

        uriData.scheme = schema;
        uriData.host = std::string( results[4].first, results[4].second );
        uriData.path = std::string( results[5].first, results[5].second );
        uriData.query = std::string( results[7].first, results[7].second );
        uriData.fragment = std::string( results[9].first, results[9].second );
        return true;
    }
};

class URI
{
public:

    URI( const std::string &uri )
       : _uri( uri ),
         _parsingIsGood( false )
    {
        URIParser parser;
        _parsingIsGood = parser.parse( _uri, _uriData );

        if( !_parsingIsGood )
            _uriData.clear();
    }

    const URIData& getURIData( ) const { return _uriData; }

    const std::string& getURI( ) const { return _uri; }

    bool isValid( ) const { return _parsingIsGood; }

private:

     std::string _uri;
     URIData _uriData;
     bool _parsingIsGood;
};

}

URI::URI( const std::string &uri )
   : _impl( new detail::URI( uri ) )
{
}

lunchbox::URI::~URI()
{
    delete _impl;
}

const std::string &URI::getScheme() const
{
    return _impl->getURIData().scheme;
}

const std::string &URI::getHost() const
{
    return _impl->getURIData().host;
}

const std::string& URI::getPath() const
{
    return _impl->getURIData().path;
}

const std::string& URI::getQuery() const
{
    return _impl->getURIData().query;
}

const std::string &URI::getFragment() const
{
    return _impl->getURIData().fragment;
}

bool URI::isEmpty() const
{
    return _impl->getURI().empty();
}

bool URI::isValid() const
{
    return _impl->isValid();
}



}
