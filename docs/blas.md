# BLAS Support

Compiling via a platform-independent IR means we will lose any machine-specific optimizations from any BLAS support. However, we can still compile CLAPACK with their reference cblas implementation.

A fork of the CLAPACK code can be found [here](https://github.com/shillaker/faasm-clapack). There are a couple of tweaks required, primarily:

- Using the Faasm toolchain
- Building the cblas library (rather than the default BLAS directory)

The cblas sources aren't part of the default CLAPACK and need to be downloaded from https://www.netlib.org/clapack/cblas.tgz (this is already done and checked into the fork).

## Building

The built archives are included in the bundled Faasm sysroot and Docker containers, so you shouldn't need to rebuild them at all.

To build from scratch:

```
cd third-party/faasm-clapack
make 
make install
```

This will install things at `/usr/local/faasm/llvm-sysroot`

## Numpy

Numpy will look for BLAS/ LAPACK at build time. 

This is done by [`distutils/system_info.py`](https://github.com/numpy/numpy/blob/master/numpy/distutils/system_info.py).

Numpy expects the cblas interface to be present, hence why we have to fiddle with CLAPACK a bit.
