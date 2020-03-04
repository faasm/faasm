# Faasm Toolchain

Although wasm support is included in upstream LLVM, we still need to build things ourselves. This will hopefully change in future.

We need to build a custom LLVM toolchain with a modified musl and libcxx and libcxxabi set to use pthreads.

To download the built toolchain you can run:

```
inv download-toolchain
```

This also includes the relevant sysroot files (which will be placed at `/usr/local/faasm/llvm-sysroot`).

A CMake toolchain file exists at `toolchain/FaasmToolchain.cmake`.

## Runtime

Certain files need to be in place at runtime, to download these:

```
inv download-runtime-root
```

# Building

## Building Toolchain

To build from scratch you just need to be in the `toolchain` directory, then run:

```
make
```

This takes ages as it builds a whole LLVM toolchain.

## Rebuilding Toolchain

To rebuild, there different options. The first is just rebuilding libc:

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

## Troubleshooting the build

If a given project fails you need to go to the build dir (e.g. for `libcxx` this would be `third-party/llvm-project/build/libcxx`) then look at `CMakeFiles/CMakeError.log` and `CMakeFiles/CMakeOutput.log`.

One thing to check for is the magic `libclang_rt.builtins-wasm32.tar.gz` file. This should get built by the build process but if you need to fiddle with it you can download it as part of the [WASI SDK Release](https://github.com/CraneStation/wasi-sdk/releases).

# Usage

## Shared Libraries

Note that at the time of writing the state of shared libraries and dynamic linking was also in flux. 

Shared libraries must be built with `-fPIC`, but this is only supported with `--target wasm32-unknown-emscripten` as per this [LLVM bug](https://bugs.llvm.org/show_bug.cgi?id=42714).

You will need to set this target explicitly the relevant CMake/ Makefile (as the default is `wasm32`).

## SIMD

SIMD support is also in flux but possible to switch on with `-msimd128` and `-munimplemented-simd128`.

## Toolchain Updates

When updating the underlying LLVM version of the toolchain you'll need to create a 
[new release](releases.md). The steps should be something like:

- Bump up the Faasm version (as outlined in [release docs](releases.md))
- Rebuild the toolchain itself (based on instructions above)
- Rebuild the basic sysroot libraries (`inv compile-libc compile-eigen compile-libfaasm`)
- Rebuild 3rd party libraries (Pyodide and Tensorflow, see relevant docs)
- Rebuild and upload _all_ wasm functions (all those under `funcs`)
- Set up the runtime root (see Python docs and Ansible `runtime_fs.yml` playbook)
- Make sure all the tests run
- Create a new [release](releases.md)
- Rebuild all Docker images for this release