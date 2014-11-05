
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef LUNCHBOX_COMPILER_H
#define LUNCHBOX_COMPILER_H

#ifdef __cplusplus
#  include <boost/config.hpp>

// C++11 features 'backported' to C++03
#  if !defined(CXX_NULLPTR_SUPPORTED) && !defined(nullptr)
#    define nullptr 0
#  endif
#  ifndef CXX_FINAL_OVERRIDE_SUPPORTED
#    ifndef final
#      define final
#    endif
#    ifndef override
#      define override
#    endif
#  endif
#endif

#ifdef _MSC_VER
#  define LB_ALIGN8( var )  __declspec (align (8)) var;
#  define LB_ALIGN16( var ) __declspec (align (16)) var;
#else
/**
 * Declare and align a variable to a 8-byte boundary.
 * @deprecated Use boost::aligned_storage
 */
#  define LB_ALIGN8( var )  var __attribute__ ((aligned (8)));
/**
 * Declare and align a variable to a 16-byte boundary.
 * @deprecated Use boost::aligned_storage
 */
#  define LB_ALIGN16( var ) var __attribute__ ((aligned (16)));
#endif

#ifdef __GNUC__
#  define LB_UNUSED __attribute__((unused))
#  ifdef WARN_DEPRECATED // Set CMake option ENABLE_WARN_DEPRECATED
#    define LB_DEPRECATED __attribute__((deprecated))
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 0)) )
#    define LB_GCC_4_0_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)) )
#    define LB_GCC_4_1_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2)) )
#    define LB_GCC_4_2_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)) )
#    define LB_GCC_4_3_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 4)) )
#    define LB_GCC_4_4_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) )
#    define LB_GCC_4_5_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) )
#    define LB_GCC_4_6_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)) )
#    define LB_GCC_4_7_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) )
#    define LB_GCC_4_8_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) )
#    define LB_GCC_4_9_OR_LATER
#  endif

#  if (( __GNUC__ < 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 3)) )
#    define LB_GCC_4_3_OR_OLDER
#  endif

#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 2))
#    define LB_GCC_4_2
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 3))
#    define LB_GCC_4_3
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 4))
#    define LB_GCC_4_4
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 5))
#    define LB_GCC_4_5
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 6))
#    define LB_GCC_4_6
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 7))
#    define LB_GCC_4_7
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 8))
#    define LB_GCC_4_8
#  endif
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 9))
#    define LB_GCC_4_9
#  endif
#endif // GCC

#ifndef LB_UNUSED
#  define LB_UNUSED
#endif

#ifdef LB_DEPRECATED
#  define LB_PUSH_DEPRECATED                                          \
    _Pragma("clang diagnostic push")                                  \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"") \
    _Pragma("GCC diagnostic push")                                    \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#  define LB_POP_DEPRECATED                                         \
    _Pragma("clang diagnostic pop")                                 \
    _Pragma("GCC diagnostic pop")
#else
#  define LB_DEPRECATED
#  define LB_PUSH_DEPRECATED
#  define LB_POP_DEPRECATED
#endif

#ifdef __GNUC__
#  define LB_LIKELY(x)       __builtin_expect( (x), 1 )
#  define LB_UNLIKELY(x)     __builtin_expect( (x), 0 )
#else
#  define LB_LIKELY(x)       x
#  define LB_UNLIKELY(x)     x
#endif

#endif //LUNCHBOX_COMPILER_H
