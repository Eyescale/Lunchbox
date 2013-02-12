# Lunchbox

Welcome to Lunchbox, a C++ library for multi-threaded
programming. Lunchbox was formerly known as eq::base or co::base, the
foundation for the Equalizer parallel rendering framework and the Collage
network library. It is intended for all application developers creating
high-performance multi-threaded programs.

## Features

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

## Downloads

* [Ubuntu Packages Repository](https://launchpad.net/~eilemann/+archive/equalizer/)
* [API Documentation](http://eyescale.github.com/)
* Building from source:

```
  git clone https://github.com/Eyescale/Buildyard.git
  cd Buildyard
  git clone https://github.com/Eyescale/config.git config.eyescale
  make Lunchbox
```

### Version 1.6

* [Source Code](http://www.equalizergraphics.com/downloads/Lunchbox-1.6.0.tar.gz)

### Version 1.4

* [Source Code](http://www.equalizergraphics.com/downloads/Lunchbox-1.4.0.tar.gz)
