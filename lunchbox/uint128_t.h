
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2014, Stefan Eilemann <eile@eyescale.ch>
 *               2010-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_UINT128_H
#define LUNCHBOX_UINT128_H

#include <lunchbox/api.h>
#include <lunchbox/compiler.h>

#include <sstream>
#ifdef _MSC_VER
// Don't include <lunchbox/types.h> to be minimally intrusive for apps
// using uint128_t
#  include <basetsd.h>
typedef UINT64     uint64_t;
#else
#  include <stdint.h>
#endif

namespace lunchbox
{
class uint128_t;
std::ostream& operator << ( std::ostream& os, const uint128_t& id );

/**
 * A base type for 128 bit unsigned integer values.
 *
 * Example: @include tests/uint128_t.cpp
 */
class uint128_t
{
public:
    /**
     * Construct a new 128 bit integer with a default value.
     * @version 1.0
     */
    explicit uint128_t( const unsigned long long low_ = 0 )
        : _high( 0 ), _low( low_ ) {}

    /**
     * Construct a new 128 bit integer with a default value.
     * @version 1.9.1
     */
    explicit uint128_t( const unsigned long low_ ) : _high( 0 ), _low( low_ ) {}

    /**
     * Construct a new 128 bit integer with a default value.
     * @version 1.9.1
     */
    explicit uint128_t( const int low_ ) : _high( 0 ), _low( low_ ) {}

#ifdef LUNCHBOX_USE_V1_API
    /**
     * Construct a new 128 bit integer with a generated universally unique
     * identifier.
     *
     * @param generate if set to false, the uint128_t will be set to 0.
     * @version 1.9.1
     * @deprecated
     */
    LUNCHBOX_API explicit uint128_t( const bool generate ) LB_DEPRECATED;
#endif

    /**
     * Construct a new 128 bit integer with default values.
     * @version 1.0
     **/
    uint128_t( const uint64_t high_, const uint64_t low_ )
        : _high( high_ ), _low( low_ ) {}

    /**
     * Construct a new 128 bit integer from a string representation.
     * @version 1.3.2
     **/
    explicit uint128_t( const std::string& string )
        : _high( 0 ), _low( 0 ) { *this = string; }

    /**
     * @return true if the uint128_t is a generated universally unique
     *         identifier.
     * @version 1.9.1
     */
    bool isUUID() const { return high() != 0; }

    /** Assign another 128 bit value. @version 1.0 */
    uint128_t& operator = ( const lunchbox::uint128_t& rhs )
        {
            _high = rhs._high;
            _low = rhs._low;
            return *this;
        }

    /** Assign another 64 bit value. @version 1.1.1 */
    uint128_t& operator = ( const uint64_t rhs )
        {
            _high = 0;
            _low = rhs;
            return *this;
        }

    /** Assign an integer value. @version 1.7.1 */
    uint128_t& operator = ( const int rhs )
        {
            _high = 0;
            _low = rhs;
            return *this;
        }

    /** Assign an 128 bit value from a std::string. @version 1.0 */
    LUNCHBOX_API uint128_t& operator = ( const std::string& from );

    /**
     * @return true if the values are equal, false if not.
     * @version 1.0
     **/
    bool operator == ( const lunchbox::uint128_t& rhs ) const
        { return _high == rhs._high && _low == rhs._low; }

    /**
     * @return true if the values are different, false otherwise.
     * @version 1.0
     **/
    bool operator != ( const lunchbox::uint128_t& rhs ) const
        { return _high != rhs._high || _low != rhs._low; }

    /**
     * @return true if the values are equal, false otherwise.
     * @version 1.9.1
     **/
    bool operator == ( const unsigned long long& low_ ) const
        { return *this == uint128_t( low_ ); }

    /**
     * @return true if the values are different, false otherwise.
     * @version 1.9.1
     **/
    bool operator != ( const unsigned long long& low_ ) const
        { return *this != uint128_t( low_ ); }

    /**
     * @return true if this value is smaller than the RHS value.
     * @version 1.0
     **/
    bool operator < ( const lunchbox::uint128_t& rhs ) const
        {
            if( _high < rhs._high )
                return true;
            if( _high > rhs._high )
                return false;
            return _low < rhs._low;
        }

    /**
     * @return true if this value is bigger than the rhs value.
     * @version 1.0
     */
    bool operator > ( const lunchbox::uint128_t& rhs ) const
        {
            if( _high > rhs._high )
                return true;
            if( _high < rhs._high )
                return false;
            return _low > rhs._low;
        }

    /**
     * @return true if this value is smaller or equal than the
     *         RHS value.
     * @version 1.0
     */
    bool operator <= ( const lunchbox::uint128_t& rhs ) const
        {
            if( _high < rhs._high )
                return true;
            if( _high > rhs._high )
                return false;
            return _low <= rhs._low;
        }

    /**
     * @return true if this value is smaller or equal than the
     *         RHS value.
     * @version 1.0
     */
    bool operator >= ( const lunchbox::uint128_t& rhs ) const
        {
            if( _high > rhs._high )
                return true;
            if( _high < rhs._high )
                return false;
            return _low >= rhs._low;
        }

    /** Increment the value. @version 1.0 */
    uint128_t& operator ++()
        {
            ++_low;
            if( !_low )
                ++_high;

            return *this;
        }

    /** Decrement the value. @version 1.0 */
    uint128_t& operator --()
        {
            if( !_low )
                --_high;
            --_low;
            return *this;
        }

    /** Add value and return the new value. @version 1.5.1 */
    uint128_t& operator +=( const lunchbox::uint128_t& rhs )
        {
            const uint64_t oldLow = _low;
            _low += rhs._low;
            if( _low < oldLow ) // overflow
                _high += rhs._high + 1;
            else
                _high += rhs._high;
            return *this;
        }

    /** @return the reference to the lower 64 bits of this 128 bit value. */
    const uint64_t& low() const { return _low; }
    /** @return the reference to the high 64 bits of this 128 bit value. */
    const uint64_t& high() const { return _high; }

    /** @return the reference to the lower 64 bits of this 128 bit value. */
    uint64_t& low() { return _low; }
    /** @return the reference to the high 64 bits of this 128 bit value. */
    uint64_t& high() { return _high; }

    /** @return a short, but not necessarily unique, string of the value. */
    std::string getShortString() const
        {
            std::stringstream stream;
            stream << std::hex << _high << _low;
            const std::string str = stream.str();
            return str.substr( 0, 3 ) + ".." +
                str.substr( str.length() - 3, std::string::npos );
        }

    /** @return the full string representation of the value. */
    std::string getString() const
        {
            std::stringstream stream;
            stream << *this;
            return stream.str();
        }

    /** Serialize this object to a boost archive. @version 1.3.1 */
    template< class Archive >
    void serialize( Archive& ar, const unsigned int /*version*/ )
    {
        ar & low();
        ar & high();
    }

#ifdef LUNCHBOX_USE_V1_API
    /** @return true if the uint128_t was generated.
     *  @deprecated
     */
    bool isGenerated() const LB_DEPRECATED { return high() != 0; }

    /** @deprecated Don't use, static initializer fiasco. Use 0/uint128_t()*/
    static LUNCHBOX_API const uint128_t ZERO LB_DEPRECATED;
#endif

private:
    uint64_t _high;
    uint64_t _low;
};

/** ostream operator for 128 bit unsigned integers. @version 1.0 */
inline std::ostream& operator << ( std::ostream& os, const uint128_t& id )
{
    if( id.high() == 0 )
        os << std::hex << id.low() << std::dec;
    else
        os << std::hex << id.high() << ':' << id.low() << std::dec;
    return os;
}

/** istream operator for 128 bit unsigned integers. @version 1.7.0 */
inline std::istream& operator >> ( std::istream& is, uint128_t& id )
{
    std::string str;
    is >> str;
    id = str;
    return is;
}

/** Add a 64 bit value to a 128 bit value. @version 1.0 */
inline uint128_t operator+ ( const lunchbox::uint128_t& a, const uint64_t& b )
{
    uint128_t result = a;
    result.low() += b;
    if( result.low() < a.low( ))
        ++result.high();
    return result;
}

/** Add two 128 bit values. @version 1.5.1 */
inline uint128_t operator+ ( const lunchbox::uint128_t& a,
                             const lunchbox::uint128_t& b )
{
    uint128_t result = a;
    result += b;
    return result;
}

/** Subtract a 64 bit value from a 128 bit value. @version 1.0 */
inline uint128_t operator- ( const lunchbox::uint128_t& a, const uint64_t& b )
{
    uint128_t result = a;
    result.low() -= b;
    if( result.low() > a.low( ))
        --result.high();
    return result;
}

/** Bitwise and operation on two 128 bit values. @version 1.1.5 */
inline uint128_t operator & ( const lunchbox::uint128_t& a,
                              const lunchbox::uint128_t& b )
{
    uint128_t result = a;
    result.high() &= b.high();
    result.low() &= b.low();
    return result;
}

/** Bitwise or operation on two 128 bit values. @version 1.1.5 */
inline uint128_t operator | ( const lunchbox::uint128_t& a,
                              const lunchbox::uint128_t& b )
{
    uint128_t result = a;
    result.high() |= b.high();
    result.low() |= b.low();
    return result;
}

/**
 * Create a 128 bit integer based on a string.
 *
 * The MD5 hash of the given text is used to form the uint128_t.
 *
 * @param string the string to form the uint128_t from.
 * @version 1.3.2
 */
LUNCHBOX_API uint128_t make_uint128( const char* string );

/**
 * Construct a new 128 bit integer with a generated universally unique
 * identifier.
 * @version 1.9.1
 */
LUNCHBOX_API uint128_t make_UUID();

}
#endif // LUNCHBOX_UINT128_H
