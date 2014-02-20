
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_DSO_H
#define LUNCHBOX_DSO_H

#include <lunchbox/api.h>
#include <lunchbox/nonCopyable.h>
#include <string>

namespace lunchbox
{
namespace detail { class DSO; }

/** Helper to access dynamic shared objects (DSO) */
class DSO : public NonCopyable
{
public:
    /** Construct a new dynamic shared object. @version 1.0 */
    LUNCHBOX_API DSO();

    /** Construct and initialize a dynamic shared object. @version 1.7.1 */
    LUNCHBOX_API explicit DSO( const std::string& name );

    /** Destruct this DSO handle. @version 1.0 */
    LUNCHBOX_API ~DSO();

    /**
     * Open a dynamic shared object.
     *
     * @param fileName The file name of the DSO.
     * @return true if the DSO was opened, false upon error.
     * @version 1.0
     */
    LUNCHBOX_API bool open( const std::string& fileName );

    /**
     * Close the DSO, invalidates retrieved function pointers.
     * @version 1.0
     */
    LUNCHBOX_API void close();

    /**
     * @return a function pointer in the DSO, or 0 if the function is not
     *         exported by the DSO.
     * @version 1.0
     */
    LUNCHBOX_API void* getFunctionPointer( const std::string& functionName);

    /**
     * @return a typed function pointer in the DSO, or 0 if the function is
     *         not exported by the DSO.
     * @version 1.7.1
     */
    template< class F > F getFunctionPointer( const std::string& func )
        { return (F)(getFunctionPointer( func )); }

    /** @return true if the DSO is loaded. @version 1.0 */
    LUNCHBOX_API bool isOpen() const;

    /**
     * @return true if both instances manage the same shared object.
     * @version 1.9.1
     */
    LUNCHBOX_API bool operator == ( const DSO& rhs );

    /**
     * @return true if both instances manage different shared objects.
     * @version 1.9.1
     */
    bool operator != ( const DSO& rhs ) { return !( *this == rhs ); }

private:
    detail::DSO* const _impl;
};

}

#endif //LUNCHBOX_DSO_H
