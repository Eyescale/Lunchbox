
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_H
#define LUNCHBOX_H


/** @mainpage Lunchbox API Documentation
    @htmlinclude "doc/RelNotes.dox"
*/

/**
 * @namespace lunchbox
 * @brief Abstraction layer and common utilities for multi-threaded programming.
 *
 * Lunchbox provides C++ classes to abstract the underlying operating system and
 * to implement common utilities for multi-threaded C++ programs. Classes with
 * non-virtual destructors are not intended to be subclassed.
 */

#include <lunchbox/any.h>
#include <lunchbox/anySerialization.h>
#include <lunchbox/api.h>
#include <lunchbox/atomic.h>
#include <lunchbox/buffer.h>
#include <lunchbox/clock.h>
#include <lunchbox/debug.h>
#include <lunchbox/file.h>
#include <lunchbox/lfVector.h>
#include <lunchbox/monitor.h>
#include <lunchbox/perThread.h>
#include <lunchbox/rng.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/sleep.h>
#include <lunchbox/spinLock.h>

#ifdef LB_SYSTEM_INCLUDES
#  include <lunchbox/os.h>
#endif

#endif // LUNCHBOX_H

