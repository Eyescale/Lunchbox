
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_PLUGIN_H
#define LUNCHBOX_PLUGIN_H

#include <functional>
#include <servus/uint128_t.h> // member

namespace lunchbox
{
/** @internal */
template <class T>
class Plugin
{
public:
    /** The constructor method for Plugin objects.  @version 1.11.0 */
    using Constructor = std::function<T*(const typename T::InitDataT&)>;

    /**
     * The method to check if the plugin can handle a given initData.
     * @version 1.11.0
     */
    using HandlesFunc = std::function<bool(const typename T::InitDataT&)>;

    /** The method to get the plugin's description. @version 1.16 */
    using DescriptionFunc = std::function<std::string()>;

    /**
     * Construct a new Plugin.
     * @param constructor The constructor method for Plugin objects.
     * @param handles_ The method to check if the plugin can handle the
     * initData.
     * @param description method to get the the help for the plugin
     * @version 1.11.0
     */
    Plugin(const Constructor& constructor, const HandlesFunc& handles_,
           const DescriptionFunc& description)
        : _constructor(constructor)
        , _handles(handles_)
        , _description(description)
    {
    }

    /** Construct a new plugin instance. @version 1.14 */
    T* construct(const typename T::InitDataT& data) const
    {
        return _constructor(data);
    }

    /** @return true if this plugin handles the given request. @version 1.14 */
    bool handles(const typename T::InitDataT& data) const
    {
        return _handles(data);
    }

    /** @return the plugin's description. @version 1.17 */
    std::string getDescription() const { return _description(); }
    bool operator==(const Plugin& rhs) const // TEST
    {
        return &_constructor == &rhs._constructor &&
               &_handles == &rhs._handles &&
               _description() == rhs._description();
    }

    bool operator!=(const Plugin& rhs) const // TEST
    {
        return !(*this == rhs);
    }

private:
    Constructor _constructor;
    HandlesFunc _handles;
    DescriptionFunc _description;
};
}

#endif
