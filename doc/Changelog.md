# Changelog


# git master {#master}

* [216]()https://github.com/Eyescale/Lunchbox/pull/216:
  getLibraryPaths: Fixes library path results for OSX and linux
* [213](https://github.com/Eyescale/Lunchbox/pull/213):
  getExecutablePath: More sensible behaviour for OS X app bundles

# Release 1.11 (07-07-2015) {#Release111}

* [208](https://github.com/Eyescale/Lunchbox/pull/208),
  [207](https://github.com/Eyescale/Lunchbox/pull/207):
  Pretty log print and denoise
* [206](https://github.com/Eyescale/Lunchbox/pull/206):
  Change log level semantics
* [205](https://github.com/Eyescale/Lunchbox/pull/205):
  Clean up lunchbox::RNG impl, removed reseed()
* [204](https://github.com/Eyescale/Lunchbox/pull/204):
  Added some power of two MB sizes defines
* [200](https://github.com/Eyescale/Lunchbox/pull/200):
  Removed uint128_t, URI and Servus from API V2 to
  [Servus library](https://github.com/HBPVIS/Servus)
* [193](https://github.com/Eyescale/Lunchbox/pull/193):
  lunchbox::PersistentMap Improvements
* [189](https://github.com/Eyescale/Lunchbox/pull/189):
  Fix [187](https://github.com/Eyescale/Lunchbox/pull/187):
  Package CMake modules in the release tar-ball
* [0026561](https://github.com/Eyescale/Lunchbox/commit/0026561):
  Added lunchbox::OMP::getThreadNum
* [178](https://github.com/Eyescale/Lunchbox/pull/178),
  [167](https://github.com/Eyescale/Lunchbox/pull/167),
  [156](https://github.com/Eyescale/Lunchbox/pull/156):
  lunchbox::PluginFactory, a mechanism to register and load plugins
* [177](https://github.com/Eyescale/Lunchbox/pull/177):
  Implement lunchbox::getLibraryPath()
* [173](https://github.com/Eyescale/Lunchbox/pull/173):
  Move (de)compressor & up/downloader to
  [Pression library](https://github.com/Eyescale/Pression)
* [172](https://github.com/Eyescale/Lunchbox/pull/172):
  Fix for lunchbox::demangleTypeID
* [171](https://github.com/Eyescale/Lunchbox/pull/171):
  Add log topic to log potential bugs
* [163](https://github.com/Eyescale/Lunchbox/pull/163):
  LB_LITTLE/BIGENDIAN defines defines
* [158](https://github.com/Eyescale/Lunchbox/pull/158):
  Added new comparison methods to Monitor
* [155](https://github.com/Eyescale/Lunchbox/pull/155):
  Add lunchbox::getExecutablePath() to find resources based on installation
  folder

# Release 1.10 (09-10-2014) {#Release110}

* Added API for continuous browsing to lunchbox::Servus
* Added lunchbox::getHostname function
* Added lunchbox::PersistentMap interface for persistent key-value stores
* Added lunchbox::URI parsing support according to RFC3986
* Added timeout parameter for the Future::wait function
* Improved lunchbox::Servus implementations, supporting continuous browsing and
  using the native Avahi API on Linux
* Improved performance of lunchbox::make_uint128()
* Improved thread logging facilities
* OSX 10.9, Win32 builds improved

# Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/Eyescale/Lunchbox/issues) if you find
any other issue with this release.

* [6:](https://github.com/Eyescale/Lunchbox/issues/6)
Condition::timedWait does not wait on early RHEL 6.1 versions
