
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2013, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_DEBUG_H
#define LUNCHBOX_DEBUG_H

#include <lunchbox/defines.h>
#include <lunchbox/log.h>

#include <typeinfo>

// assertions
// #define LB_RELEASE_ASSERT

namespace lunchbox
{
/**
 * @internal
 * Used to trap into an infinite loop to allow debugging of assertions
 */
LUNCHBOX_API void abort(bool dumpThreads = false);

/**
 * @internal
 * Check the consistency of the heap and abort on error (Win32 only).
 */
LUNCHBOX_API void checkHeap();

/**
 * Print a textual description of the current system error.
 *
 * The current system error is OS-specific, e.g., errno or GetLastError().
 * @version 1.0
 */
LUNCHBOX_API std::ostream& sysError(std::ostream& os);

/** @return a textual description of the current system error. @version 1.9.1 */
LUNCHBOX_API std::string sysError();

/**
 * Get the current call stack.
 *
 * May not be implemented on all platforms.
 *
 * @param skipFrames the number of most recent stack frames to ignore.
 * @version 1.9.1
 */
LUNCHBOX_API std::string backtrace(const size_t skipFrames);

/** Print the current call stack. @version 1.0 */
LUNCHBOX_API std::ostream& backtrace(std::ostream& os);

LUNCHBOX_API std::string demangleTypeID(const char* mangled); //!< @internal

#ifdef _WIN32
#pragma warning(disable : 4100) // VS Bug
#endif
/** Print the RTTI name of the given class. @version 1.0 */
template <class T>
inline std::string className(const T& object)
{
    return demangleTypeID(typeid(object).name());
}

/** Print the RTTI name of the given class. @version 1.0 */
template <class T>
inline std::string className(const T* object)
{
    return className(*object);
}

#ifdef _WIN32
#pragma warning(default : 4100)
#endif

/**
 * Format the given array in a human-readable form.
 * Depending on the data type, a different formatting may be used.
 *
 * @param data The pointer to the data to print.
 * @param num The number of elements of T to print, for T==void the number of
 *            bytes.
 * @version 1.9.1
 */
template <class T>
inline std::string format(const T* data, const size_t num)
{
    std::ostringstream os;
    os << num << " " << className(data) << " @ " << std::hex
       << (const void*)data << ": 0x";
    for (size_t i = 0; i < num; ++i)
    {
        if ((i % 8) == 0)
            os << "   " << std::endl;
        os << ' ' << data[i];
    }
    return os.str();
}

template <>
inline std::string format(const uint8_t* data, const size_t num)
{
    std::ostringstream os;
    os << num << " bytes @ " << std::hex << (const void*)data << ": 0x";
    os.precision(2);
    for (size_t i = 0; i < num; ++i)
    {
        if ((i % 32) == 0)
            os << "   " << std::endl;
        else if ((i % 8) == 0)
            os << ' ';
        os << ' ' << std::setw(2) << int(data[i]);
    }
    return os.str();
}

template <>
inline std::string format(const void* data, const size_t num)
{
    return format(reinterpret_cast<const uint8_t*>(data), num);
}

template <class T>
inline std::string format(const std::vector<T>& data)
{
    return format<T>(&data[0], data.size());
}

} // namespace lunchbox

#ifdef NDEBUG
#ifdef LB_RELEASE_ASSERT
#define LBASSERT(x)                                                    \
    {                                                                  \
        if (!(x))                                                      \
            LBERROR << "##### Assert: " << #x << " #####" << std::endl \
                    << lunchbox::forceFlush;                           \
        lunchbox::checkHeap();                                         \
    }
#define LBASSERTINFO(x, info)                                              \
    {                                                                      \
        if (!(x))                                                          \
            LBERROR << "##### Assert: " << #x << " [" << info << "] #####" \
                    << std::endl                                           \
                    << lunchbox::forceFlush;                               \
        lunchbox::checkHeap();                                             \
    }
#define LBCHECK(x)             \
    {                          \
        const bool eqOk = x;   \
        LBASSERTINFO(eqOk, #x) \
    }
#else
#define LBASSERT(x)
#define LBASSERTINFO(x, info)
#define LBCHECK(x) \
    {              \
        x;         \
    }
#endif

#define LBUNIMPLEMENTED                                                    \
    {                                                                      \
        LBERROR << "Unimplemented code in " << __FILE__ << ":" << __LINE__ \
                << std::endl                                               \
                << lunchbox::forceFlush;                                   \
    }
#define LBUNREACHABLE                                                    \
    {                                                                    \
        LBERROR << "Unreachable code in " << __FILE__ << ":" << __LINE__ \
                << std::endl                                             \
                << lunchbox::forceFlush;                                 \
    }
#define LBDONTCALL                                                     \
    {                                                                  \
        LBERROR << "Code is not supposed to be called in this context" \
                << std::endl                                           \
                << lunchbox::forceFlush;                               \
    }
#define LBABORT(info)                                               \
    {                                                               \
        LBERROR << "##### Abort: " << info << " #####" << std::endl \
                << lunchbox::forceFlush;                            \
    }

#else // NDEBUG

#define LBASSERT(x)                             \
    {                                           \
        if (!(x))                               \
        {                                       \
            LBERROR << "Assert: " << #x << " "; \
            lunchbox::abort();                  \
        }                                       \
        lunchbox::checkHeap();                  \
    }
#define LBASSERTINFO(x, info)                                    \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            LBERROR << "Assert: " << #x << " [" << info << "] "; \
            lunchbox::abort();                                   \
        }                                                        \
        lunchbox::checkHeap();                                   \
    }

#define LBUNIMPLEMENTED                                                    \
    {                                                                      \
        LBERROR << "Unimplemented code in " << __FILE__ << ":" << __LINE__ \
                << " " << lunchbox::className(*this);                      \
        lunchbox::abort();                                                 \
    }
#define LBUNREACHABLE                                                    \
    {                                                                    \
        LBERROR << "Unreachable code in " << __FILE__ << ":" << __LINE__ \
                << " " << lunchbox::className(*this);                    \
        lunchbox::abort();                                               \
    }
#define LBDONTCALL                                                            \
    {                                                                         \
        LBERROR << "Code is not supposed to be called in this context, type " \
                << lunchbox::className(*this);                                \
        lunchbox::abort();                                                    \
    }

#define LBCHECK(x)             \
    {                          \
        const bool eqOk = x;   \
        LBASSERTINFO(eqOk, #x) \
    }
#define LBABORT(info)                 \
    {                                 \
        LBERROR << "Abort: " << info; \
        lunchbox::abort();            \
    }
#endif // NDEBUG

#define LBSAFECAST(to, in) \
    static_cast<to>(in);   \
    LBASSERT(in == 0 || dynamic_cast<to>(static_cast<to>(in)))

#endif // LUNCHBOX_DEBUG_H
