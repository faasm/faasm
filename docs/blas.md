# BLAS Support

Without a Fortran-wasm compiler we can't use standard LAPACK and BLAS libraries,
so instead we use [CLAPACK](http://www.netlib.org/clapack/). 

We use [this fork](https://github.com/faasm/faasm-clapack) with a couple of
changes to cross-compile to wasm.

CBLAS sources aren't part of the default CLAPACK and need to be downloaded from
https://www.netlib.org/clapack/cblas.tgz (this is already done and checked into
the fork).

## Building

The built archives are included in the standard Faasm sysroot so you should
only need to rebuild them if changing the toolchain or the CLAPACK code.

To build, run:

```bash
inv libs.clapack
```

