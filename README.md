[TOC]

# Introduction {#Introduction}

Welcome to Lunchbox, a C++ library for multi-threaded programming. Lunchbox was
formerly known as eq::base or co::base, the foundation for the
[Equalizer parallel rendering framework](https://github.com/Eyescale/Equalizer)
and the [Collage network library](https://github.com/Eyescale/Collage). It is
intended for all application developers creating high-performance multi-threaded
programs.

The
[API documentation](http://eyescale.github.io/Lunchbox-1.11/index.html) can be
found on [eyescale.github.io](http://eyescale.github.io/). As with any open
source project, the available source code, in particular the
[unit tests](https://github.com/Eyescale/Lunchbox/tree/1.11/tests) provide a
reference for developing applications.

Technical questions can be posted to the Equalizer
[Developer Mailing List](http://www.equalizergraphics.com/cgi-bin/mailman/listinfo/eq-dev),
or directly to
[info@equalizergraphics.com](mailto:info@equalizergraphics.com?subject=Lunchbox%20question).

Commercial support, custom software development and porting services are
available from [Eyescale](http://www.eyescale.ch). Please contact
[info@eyescale.ch](mailto:info@eyescale.ch?subject=Lunchbox%20support)
for further information.

# Features {#Features}

Lunchbox provides the following major features to facilitate the
development and deployment of multi-threaded applications:
* Operating System Abstraction: lunchbox::Atomic, lunchbox::Condition,
  lunchbox::DSO, @ref bitops "bit operations", lunchbox::daemonize(),
  (lunchbox::Clock, lunchbox::MemoryMap, lunchbox::PerThread, lunchbox::RNG,
  lunchbox::Thread)
* High-Performance Threading Primitives: lunchbox::Buffer, lunchbox::LFQueue,
  lunchbox::LFVector, lunchbox::Monitor, lunchbox::MTQueue,
  lunchbox::RequestHandler, lunchbox::SpinLock, (lunchbox::Lock,
  lunchbox::TimedLock)
* Utility classes: lunchbox::Any, lunchbox::Log, lunchbox::Pool,
  lunchbox::UnorderedIntervalSet, lunchbox::Future, lunchbox::PersistentMap,
  (lunchbox::ScopedMutex)

# Building {#Building}

Lunchbox is a cross-platform library, designed to run on any modern operating
system, including all Unix variants and the Windows operating system. Lunchbox
uses CMake to create a platform-specific build environment. The following
platforms and build environments are tested:

* Linux: Ubuntu 14.04, RHEL 6.6 (Makefile, Ninja)
* Windows: 7 (Visual Studio 2012)
* Mac OS X: 10.8 (Makefile, Ninja)

~~~
git clone https://github.com/Eyescale/Lunchbox.git
cd Lunchbox
mkdir build
cd build
cmake ..
make
~~~
