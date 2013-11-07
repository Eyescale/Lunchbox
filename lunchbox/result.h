
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_RESULT_H
#define LUNCHBOX_RESULT_H

#include <lunchbox/api.h>
#include <lunchbox/log.h>
#include <lunchbox/types.h>

#include <iostream>

namespace lunchbox
{

/** A result returns an error code and behaves like a boolean. */
class Result
{
    typedef void (Result::*bool_t)() const;
    void bool_true() const {}

public:
    static const int32_t SUCCESS = 0;

    /** Construct a new result. @version 1.9.1 */
    explicit Result( const int32_t code ) : code_( code ){}

    /** Destruct the result. @version 1.9.1 */
    virtual ~Result(){}

    /** @return true if no error occured, false otherwise. @version 1.9.1 */
    operator bool_t() const { return code_ == SUCCESS ? &Result::bool_true : 0;}

    /** @return true if an error operator, false otherwise. @version 1.9.1 */
    bool operator !() const { return code_ == SUCCESS ? 0 : &Result::bool_true;}

    /** @return true if the result is equal to the given value. @version 1.9.1*/
    bool operator == ( const int32_t code ) const { return code_ == code; }

    /** @return true if the result is not equal to the rhs. @version 1.9.1*/
    bool operator != ( const int32_t code ) const { return code != code_; }

    /** @return the result code. @version 1.9.1 */
    int32_t getCode() const { return code_; }

    /** @return the result string. @version 1.9.1 */
    virtual std::string getString() const
        { return code_ == SUCCESS ? "success" : "result"; }

protected:
    int32_t code_;
};

inline std::ostream& operator << ( std::ostream& os, const Result& result )
{
    return os << result.getString() << " (" << result.getCode() << ")";
}

}
#endif //LUNCHBOX_RESULT_H
