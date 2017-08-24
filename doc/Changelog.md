# Changelog {#Changelog}

# git master
* [317](https://github.com/Eyescale/Lunchbox/pull/317):
  Add app-global ThreadPool support

# Release 1.16 (23-05-2017)

* [310](https://github.com/Eyescale/Lunchbox/pull/310):
  Fix static initializers in Log
* [304](https://github.com/Eyescale/Lunchbox/pull/304):
  Remove Condition, functionality is covered by std::condition_variable
* [303](https://github.com/Eyescale/Lunchbox/pull/303):
  Remove TimedLock, functionality is covered by std::timed_mutex
* [302](https://github.com/Eyescale/Lunchbox/pull/302):
  Remove Lock, functionality is covered by std::mutex
* [301](https://github.com/Eyescale/Lunchbox/pull/301):
  Rename Launcher to standalone fork() function
* [300](https://github.com/Eyescale/Lunchbox/pull/300):
  Remove stdExt.h, functionality is covered by std::unordered_map
* [298](https://github.com/Eyescale/Lunchbox/pull/298):
  Add MemoryMap::resize()
* [294](https://github.com/Eyescale/Lunchbox/pull/294):
  * Add description to plugins
  * Add lunchbox::string::prepend()
  * Add lunchbox::term::getSize()
* [292](https://github.com/Eyescale/Lunchbox/pull/292):
  Add MemoryMap element getter
* [290](https://github.com/Eyescale/Lunchbox/pull/290):
  A ThreadPool class

# Release 1.15 (09-12-2016)

* [283](https://github.com/Eyescale/Lunchbox/pull/283):
  Break PersistentMap out into keyv::Map
* [278](https://github.com/Eyescale/Lunchbox/pull/278):
  Simplified Plugin API
* [276](https://github.com/Eyescale/Lunchbox/pull/276):
  Renamed UnorderedIntervalSet to IntervalSet
* [275](https://github.com/Eyescale/Lunchbox/pull/275):
  Remove obsolete MPI and OpenMP classes
* [272](https://github.com/Eyescale/Lunchbox/pull/272):
  Add getWorkDir()
* [271](https://github.com/Eyescale/Lunchbox/pull/271):
  Add '--lb-logfile' command line option
* Removed obsolete SKV backend for PersistentMap
* [266](https://github.com/Eyescale/Lunchbox/pull/266):
  Add PersistentMap::getValues and PersistentMap::takeValues

# Release 1.14 (30-06-2016)

* [263](https://github.com/Eyescale/Lunchbox/pull/263):
  Add memcached PersistentMap backend, add
  PersistentMap::createCache
* [252](https://github.com/Eyescale/Lunchbox/pull/252):
  Monitor::set() returns old value

# Release 1.13 (07-03-2016)

* [249](https://github.com/Eyescale/Lunchbox/pull/249):
  load/save binary/ascii for servus::Serializable
* [239](https://github.com/Eyescale/Lunchbox/pull/239):
  Fixed a potential buffer overrun bug in getFilename

# Release 1.12 (02-11-2015)

* [233](https://github.com/Eyescale/Lunchbox/pull/233):
  Fix RequestHandler::waitRequest to not unregister the request if it
  times out, Request::relinquish() replaced by Request::unregister()
* [218](https://github.com/Eyescale/Lunchbox/pull/218):
  Fix write to lunchbox::MemoryMap on MSVC
* [218](https://github.com/Eyescale/Lunchbox/pull/218):
  Fix missing file:line print in backtrace() on MSVC
* [217](https://github.com/Eyescale/Lunchbox/pull/217):
  Implement lunchbox::getRootPath()
* [216](https://github.com/Eyescale/Lunchbox/pull/216):
  getLibraryPaths: Fixes library path results for OSX and linux
* [213](https://github.com/Eyescale/Lunchbox/pull/213):
  getExecutablePath: More sensible behaviour for OS X app bundles

# Release 1.11 (07-07-2015)

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

# Release 1.10 (09-10-2014)

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
