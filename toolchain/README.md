# Faasm Toolchain

Although wasm support is included in upstream LLVM, we still need to build things ourselves. We need a wasm-friendly libcxx and libc with threading support, which is unfortunately not included in the standard Emscripten/ wasi-sdk approach.

To do this we need to build a custom LLVM toolchain with a modified musl and libcxx and libcxxabi set to use pthreads.

## Shared Libraries

Note that at the time of writing the state of shared libraries and dynamic linking was also in flux. 

Shared libraries must be built with `-fPIC`, but this is only supported with `--target wasm32-unknown-emscripten` as per this [LLVM bug](https://bugs.llvm.org/show_bug.cgi?id=42714).

You will need to set this target explicitly the relevant CMake/ Makefile (as the default is `wasm32`).

## Building

To build from scratch you just need to be in the `toolchain` directory, then run:

```
make
```

This takes ages as it builds a whole LLVM toolchain.

## Rebuilding

To rebuild, there differet options. The first is just rebuilding libc:

```
make clean-libc
make
```

Another is rebuilding all the libs, i.e. libc, libcxx, libcxxabi and compiler-rt:

```
make clean-libs
make
```

The final option is to rebuild EVERYTHING, including Clang. This takes ages and is only necessary if you need to change the underlying LLVM or Clang configuration (rare):

```
make clean-all
make
```


