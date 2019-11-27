# BLAS Support

The main blocker on BLAS support is a lack of Fortran-wasm compiler. A secondary issue is the loss of any machine-specific optimizations when compiling anything via wasm. 

Although no longer officially maintained, [CLAPACK](http://www.netlib.org/clapack/) can still be used to provide the relevant routines where necessary. 

A fork of the CLAPACK code can be found [here](https://github.com/shillaker/faasm-clapack). There are a couple of tweaks required, primarily:

- Using the Faasm toolchain
- Building the cblas library (rather than the default BLAS directory)
- Interfacing properly between cblas and the f2c'd code

Cblas sources aren't part of the default CLAPACK and need to be downloaded from https://www.netlib.org/clapack/cblas.tgz (this is already done and checked into the fork).

## Building

The built archives are included in the bundled Faasm sysroot and Docker containers, so you shouldn't need to rebuild them at all.

To build from scratch:

```bash
cd third-party/faasm-clapack
make 
make install
```

This will install things at `/usr/local/faasm/llvm-sysroot`. 

## Numpy

When no system LAPACK is present, Numpy will use its own f2c'd code. This is configured [here](https://github.com/numpy/numpy/blob/master/numpy/linalg/setup.py) and is from their `lapack_lite` module. 

To detect a system LAPACK, Numpy will look for different BLAS/ LAPACK libraries at build time via [`distutils/system_info.py`](https://github.com/numpy/numpy/blob/master/numpy/distutils/system_info.py).

Numpy expects a cblas interface to be present.

### Configuring Numpy

As described in [`system_info.py`](https://github.com/numpy/numpy/blob/master/numpy/distutils/system_info.py#L48) numpy looks for extra info in certain locations like `~/.numpy-site.cfg` and environment variables. 

The following builds and installs numpy without any BLAS support:

```bash
./bin/build_unoptimized_numpy.sh
```

You can check the results with:

```python
import numpy as np

# This should show all as NOT_AVAILABLE
np.__config__.show()

# This should take several seconds and only use one thread
a = np.random.rand(2048, 2048)
b = np.random.rand(2048, 2048)
c = np.dot(a, b)
```

You can then try running the same thing with a default installation. There should be some info under `lapack_opt_info` and `blas_opt_info`, then the matrix multiplication will take under a second.
