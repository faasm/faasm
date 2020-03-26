# Faasm Toolchain

Faasm aims to support a range of legacy applications, so requires a toolchain
capable of compiling large projects that may require threading, C++ exceptions 
and dynamic linking. Unfortunately these features are not covered by the 
[wasi-sdk](https://github.com/WebAssembly/wasi-sdk) and 
[wasi-libc](https://github.com/WebAssembly/wasi-libc) at this time, therefore
we need to use a custom LLVM toolchain and musl fork. The musl fork is based
off the now-archived project found [here](https://github.com/jfbastien/musl). 

Fortunately we only rarely need to build the toolchain, and it can be downloaded
by running:

```bash
# Toolchain - clang, clang++, wasm-ld etc.
inv toolchain.download-toolchain

# Sysroot - wasm-compiled libc, libcxx etc.
inv toolchain.download-sysroot
```

This repo contains a [Faasm CMake toolchain](../toolchain/FaasmToolchain.cmake),
file that's used under the hood to build functions and libraries.

# Building

On the rare occasion that we do need to rebuild the toolchain (e.g. when a new 
LLVM version is released), we need to do the following:

## Upgrading LLVM

To upgrade the underlying LLVM version you need to update the _submodule_ in this 
project. Find the commit related to tag name for the desired release in 
[the LLVM repo](https://github.com/llvm/llvm-project/releases) (e.g. `llvmorg-10.0.0`), then:

```bash
cd third-party/llvm-project
git checkout master
git fetch origin
git checkout <tag-name>
```

Once done, check the [toolchain Makefile](../toolchain/Makefile) for any references to LLVM 
versions and update accordingly.

You can then follow the steps below as normal, making sure you start with `make clean-all`.

## Building Toolchain

To build from scratch you just need to be in the `toolchain` directory, then run:

```
make
```

This a _very_ long time as it builds a lot of the LLVM toolchain from scratch.

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
- Rebuild the basic sysroot libraries (`inv libs.libc libs.eigen libs.faasm`)
- Rebuild 3rd party libraries (Pyodide and Tensorflow, see relevant docs)
- Rebuild and upload _all_ wasm functions (all those under `funcs`)
- Set up the runtime root (see Python docs and Ansible `runtime_fs.yml` playbook)
- Make sure all the tests run
- Create a new [release](releases.md)
- Rebuild all Docker images for this release