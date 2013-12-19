
/* Copyright (c) 2010-2013, Stefan Eilemann <eile@eyescale.ch>
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
 *
 * Welcome to Lunchbox, a C++ library for multi-threaded programming. Lunchbox
 * provides the following functionality:
 *
 * - Operating System Abstraction: lunchbox::Atomic, lunchbox::Condition,
 *   lunchbox::DSO, @ref bitops "bit operations", lunchbox::daemonize(),
 *   (lunchbox::Clock, lunchbox::MemoryMap, lunchbox::PerThread, lunchbox::RNG,
 *   lunchbox::Thread)
 * - High-Performance Threading Primitives: lunchbox::Buffer, lunchbox::LFQueue,
 *   lunchbox::LFVector, lunchbox::Monitor, lunchbox::MTQueue,
 *   lunchbox::RequestHandler, lunchbox::SpinLock, (lunchbox::Lock,
 *   lunchbox::TimedLock)
 * - Utility classes: lunchbox::Any, lunchbox::Log, lunchbox::Pool,
 *   lunchbox::uint128_t, lunchbox::UnorderedIntervalSet, lunchbox::Future,
 *   lunchbox::Servus, (lunchbox::ScopedMutex)
 * - Compression and CPU-GPU transfer plugin handling: lunchbox::Compressor,
 *   lunchbox::Decompressor, lunchbox::Downloader, lunchbox::Plugin,
 *   lunchbox::PluginRegistry, lunchbox::Uploader
 *
 * @ref RelNotes
 */

/**
 * @namespace lunchbox
 * @brief Abstraction layer and common utilities for multi-threaded programming.
 *
 * Lunchbox provides C++ classes to abstract the underlying operating system and
 * to implement common utilities for multi-threaded C++ programs. Classes with
 * non-virtual destructors are not intended to be subclassed.
 */

#include <lunchbox/plugins/compressor.h>

#include <lunchbox/algorithm.h>
#include <lunchbox/any.h>
#include <lunchbox/api.h>
#include <lunchbox/atomic.h>
#include <lunchbox/buffer.h>
#include <lunchbox/clock.h>
#include <lunchbox/daemon.h>
#include <lunchbox/debug.h>
#include <lunchbox/file.h>
#include <lunchbox/future.h>
#include <lunchbox/futureFunction.h>
#include <lunchbox/lfVector.h>
#include <lunchbox/memoryMap.h>
#include <lunchbox/monitor.h>
#include <lunchbox/mtQueue.h>
#include <lunchbox/perThread.h>
#include <lunchbox/referenced.h>
#include <lunchbox/refPtr.h>
#include <lunchbox/rng.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/servus.h>
#include <lunchbox/sleep.h>
#include <lunchbox/spinLock.h>
#include <lunchbox/stdExt.h>
#include <lunchbox/version.h>

#ifdef LB_SYSTEM_INCLUDES
#  include <lunchbox/os.h>
#endif

#endif // LUNCHBOX_H
