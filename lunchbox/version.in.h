
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef LUNCHBOX_VERSION_H
#define LUNCHBOX_VERSION_H

#include <lunchbox/api.h>
#include <lunchbox/base/types.h>
#include <string>

namespace lunchbox
{
    // LunchBox version macros and functions
    /** The current major version. @version 0.4 */
#   define LUNCHBOX_VERSION_MAJOR @VERSION_MAJOR@

    /** The current minor version. @version 0.4 */
#   define LUNCHBOX_VERSION_MINOR @VERSION_MINOR@

    /** The current patch level. @version 0.4 */
#   define LUNCHBOX_VERSION_PATCH @VERSION_PATCH@

    /** The current DSO binary revision. @version 0.4 */
#   define LUNCHBOX_VERSION_ABI @VERSION_ABI@

/** True if the current version is newer than the given one. @version 0.4 */
#   define LUNCHBOX_VERSION_GT( MAJOR, MINOR, PATCH )                   \
    ( (LUNCHBOX_VERSION_MAJOR>MAJOR) ||                                 \
      (LUNCHBOX_VERSION_MAJOR==MAJOR &&                                 \
       (LUNCHBOX_VERSION_MINOR>MINOR ||                                 \
        (LUNCHBOX_VERSION_MINOR==MINOR && LUNCHBOX_VERSION_PATCH>PATCH))))

/** True if the current version is equal or newer to the given. @version 0.4 */
#   define LUNCHBOX_VERSION_GE( MAJOR, MINOR, PATCH )                         \
    ( (LUNCHBOX_VERSION_MAJOR>MAJOR) ||                                       \
      (LUNCHBOX_VERSION_MAJOR==MAJOR &&                                 \
       (LUNCHBOX_VERSION_MINOR>MINOR ||                                 \
        (LUNCHBOX_VERSION_MINOR==MINOR && LUNCHBOX_VERSION_PATCH>=PATCH))))

/** True if the current version is older than the given one. @version 0.4 */
#   define LUNCHBOX_VERSION_LT( MAJOR, MINOR, PATCH )                         \
    ( (LUNCHBOX_VERSION_MAJOR<MAJOR) ||                                       \
      (LUNCHBOX_VERSION_MAJOR==MAJOR &&                                 \
       (LUNCHBOX_VERSION_MINOR<MINOR ||                                 \
        (LUNCHBOX_VERSION_MINOR==MINOR && LUNCHBOX_VERSION_PATCH<PATCH))))

/** True if the current version is older or equal to the given. @version 0.4 */
#   define LUNCHBOX_VERSION_LE( MAJOR, MINOR, PATCH )                         \
    ( (LUNCHBOX_VERSION_MAJOR<MAJOR) ||                                       \
      (LUNCHBOX_VERSION_MAJOR==MAJOR &&                                 \
       (LUNCHBOX_VERSION_MINOR<MINOR ||                                 \
        (LUNCHBOX_VERSION_MINOR==MINOR && LUNCHBOX_VERSION_PATCH<=PATCH))))

    /** Information about the current LunchBox version. */
    class LUNCHBOX_API Version
    {
    public:
        /** @return the current major version of LunchBox. @version 0.4 */
        static uint32_t getMajor();

        /** @return the current minor version of LunchBox. @version 0.4 */
        static uint32_t getMinor();

        /** @return the current patch level of LunchBox. @version 0.4 */
        static uint32_t getPatch();

        /** @return the current revision of LunchBox. @version 0.4 */
        static std::string getRevision();

        /** @return the current DSO binary revision. @version 0.4 */
        static uint32_t getABI();

        /** @return the current LunchBox version (MMmmpp). @version 0.4 */
        static uint32_t getInt();

        /** @return the current LunchBox version (MM.mmpp). @version 0.4 */
        static float    getFloat();

        /**
         * @return the current LunchBox version (MM.mm.pp[.rr]).
         * @version 1.1
         */
        static std::string getString();

        /**
         * @return true if the link-time and compile-time DSO versions are
         *         compatible.
         */ 
        static bool check()
        { return getMajor()==LUNCHBOX_VERSION_MAJOR && getMinor()==LUNCHBOX_VERSION_MINOR; }
    };
}

#endif //LUNCHBOX_VERSION_H
