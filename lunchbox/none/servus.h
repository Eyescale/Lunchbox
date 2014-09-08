
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

namespace lunchbox
{
namespace none
{
static const std::string empty_;

class Servus
{
public:
    Servus() {}
    virtual ~Servus() {}
    void set( const std::string& key, const std::string& ) final {}
    Strings getKeys() const final { return Strings(); }
    const std::string& get( const std::string& ) const final
    {
        return empty_;
    }

    lunchbox::Servus::Result announce( const unsigned short,
                                       const std::string& ) final
    {
        return lunchbox::Servus::Result(
            lunchbox::Servus::Result::NOT_SUPPORTED );
    }

    void withdraw() final {}
    bool isAnnounced() const final { return false; }

    lunchbox::Servus::Result beginBrowsing(
        const lunchbox::Servus::Interface ) final
    {
        return lunchbox::Servus::Result(
            lunchbox::Servus::Result::NOT_SUPPORTED );
    }

    lunchbox::Servus::Result browse( const int32_t ) final
    {
        return lunchbox::Servus::Result(
            lunchbox::Servus::Result::NOT_SUPPORTED );
    }

    void endBrowsing() final {}
    bool isBrowsing() const final { return false; }
    Strings discover( const lunchbox::Servus::Interface interface_,
                      const unsigned browseTime ) final
    {
        return getInstances();
    }

    Strings getInstances() const final { return Strings; }
    Strings getKeys( const std::string& instance ) const final { return Strings(); }
    bool containsKey( const std::string& instance,
                      const std::string& key ) const final { return false; }
    const std::string& get( const std::string& instance,
                            const std::string& key ) const final
    {
        return empty_;
    }

    void getData( lunchbox::Servus::Data& data ) final {}
};
}
}

#endif
