Release Notes {#RelNotes}
============

[TOC]

# Introduction {#Introduction}

Welcome to Lunchbox, a C++ library for multi-threaded programming.
Lunchbox 1.9 adds MPI support, fixes for compilation on different platforms, 
resolves deprecation warnings and adds minor improvements over the API.

Lunchbox 1.9 is a feature release based on the 1.8 Lunchbox API. It is
intended for all application developers creating high-preformance
multi-threaded programs. Lunchbox 1.9 can be retrieved by downloading
the
[source code](http://www.equalizergraphics.com/downloads/Lunchbox-1.9.0.tar.gz)
or any of the
[precompiled packages](http://www.equalizergraphics.com/downloads/major.html).

## Features {#Features}

Lunchbox provides the following major features to facilitate the
development and deployment of multi-threaded applications:

* OS Abstraction: Utility classes abstracting common operating systems
  features, e.g., threads, locks, memory maps, shared library loading
  and condition variables.
* High-Performance Primitives: Thread-safe utilities tuned for
  performance, e.g., atomic variables, spin locks and lock-free
  containers.
* Utility classes: Helper primitives which are not in the standard
  library, e.g., logging, pools and random number generation.

# New in this release {#New}

Lunchbox 1.9 contains the following features, enhancements, bug fixes
and documentation changes over Lunchbox 1.8:

## New Features {#NewFeatures}

* MPI support added through the new MPI class.

## Enhancements {#Enhancements}

* Added timeout parameter for the Future::wait function
* Improved thread logging facilities
* Structures added to gather compression results
* Boolean operators added for (De)compressor
* Array has been moved from Collage to Lunchbox

## Optimizations {#Optimizations}

## Documentation {#Documentation}

* [API documentation](http://eyescale.github.io/Lunchbox-1.9/index.html)
  of the public Lunchbox API

## Bug Fixes {#Fixes}

Lunchbox 1.9 includes various compilation fixes over the 1.8 release, including
the following:

* OSX 10.9, Win32 builds fixed
* Byteswap for unsigned long is satisfied on clang, on OSX
* Deprecation warnings resolved

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/Eyescale/Lunchbox/issues) if you find
any other issue with this release.

* [6:](https://github.com/Eyescale/Lunchbox/issues/6)
Condition::timedWait does not wait on early RHEL 6.1 versions

# About {#About}

Lunchbox is a cross-platform library, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. Lunchbox uses CMake to create a platform-specific build
environment. The following platforms and build environments are tested:

* Linux: Ubuntu 12.04, 12.10, 13.04, 13.10 RHEL 6 (Makefile, i386, x64)
* Windows: 7 (Visual Studio 2008, i386, x64)
* Mac OS X: 10.8 (Makefile, XCode, i386, x64)

The
[API documentation](http://eyescale.github.io/Lunchbox-1.8/index.html)
can be found on [eyescale.github.io](http://eyescale.github.io/). As
with any open source project, the available source code, in particular
the [unit tests](https://github.com/Eyescale/Lunchbox/tree/1.8/tests)
provide a reference for developing applications.

Technical questions can be posted to the Equalizer
  [Developer Mailing List](http://www.equalizergraphics.com/cgi-bin/mailman/listinfo/eq-dev),
  or directly to
  [info@equalizergraphics.com](mailto:info@equalizergraphics.com?subject=Lunchbox%20question).

Commercial support, custom software development and porting services are
available from [Eyescale](http://www.eyescale.ch). Please contact
[info@eyescale.ch](mailto:info@eyescale.ch?subject=Lunchbox%20support)
for further information.

# Errata
