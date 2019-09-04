# Faasm Toolchain

This is in flux at the moment.

We need a wasm-friendly libcxx and libc with pthreads on. This is a little different to the standard Emscripten/ wasi-sdk approach where threading is turned off.

Unfortunately to do this we need to build a custom LLVM toolchain with a modified musl and libcxx and libcxxabi set to use pthreads.

## Shared Libraries

Note that at the time of writing the state of shared libraries and dynamic linking was also in flux. 

Shared libraries must be built with `-shared -fPIC -nostdlib` to avoid linking in any of the standard library.

They must also set `-target wasm32-unknown-emscripten` as per this [LLVM bug](https://bugs.llvm.org/show_bug.cgi?id=42714).

You will need to override this in the relevant CMake/ Makefile (as the default is `wasm32`).

## Building

To build from scratch you just need to be in this directory, then run:

```
make
```

This takes ages as it builds a whole LLVM toolchain.

To rebuild, there are two options. The first is just rebuilding the libs, i.e. libcxx, libc etc.

```
make clean-libs
make
```

The second is to rebuild EVERYTHING, including Clang. This takes ages and is only necessary if you need to change the underlying LLVM or Clang configuration (rare):

```
make clean-all
make
```


