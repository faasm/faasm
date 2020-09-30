# BLAS Support

Without a Fortran-wasm compiler we can't use standard LAPACK and BLAS libraries,
so instead we use [CLAPACK](http://www.netlib.org/clapack/). 

The forked code is found [here](https://github.com/faasm/faasm-clapack) with a 
couple of changes to cross-compile to wasm.

## Building

The built archives are included in the standard Faasm sysroot so you should
only need to rebuild them if changing the toolchain or the CLAPACK code.

To build, run:

```bash
inv libs.clapack
```

