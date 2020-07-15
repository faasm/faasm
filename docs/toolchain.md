# Faasm Toolchain

Faasm aims to support a range of legacy applications, so requires a toolchain
capable of compiling large projects that may require threading, C++ exceptions 
and dynamic linking. To do this we have to use a custom 
[fork of wasi-libc](https://github.com/Shillaker/wasi-libc), and our own build of 
the LLVM toolchain via [this Makefile](../toolchain/Makefile).  

# Usage

## Dynamic Linking

Faasm implements dynamic linking according to the 
[WebAssembly tool conventions](https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md). 

Shared libraries must be built with `-fPIC`, but this is only supported with 
`--target wasm32-unknown-emscripten` as per this [LLVM bug](https://bugs.llvm.org/show_bug.cgi?id=42714).

You will need to set this target explicitly in any relevant CMakeLists/ Makefiles (as the default is `wasm32`).

## SIMD

SIMD support is switched on using the standard Clang flags, `-msimd128` and `-munimplemented-simd128`.
This is done by default for all Faasm functions and libraries. 

At time of writing the 
[Clang WASM SIMD header](https://github.com/llvm/llvm-project/blob/master/clang/lib/Headers/wasm_simd128.h) 
was not found in Clang, so we put our own copy into our wasi-libc fork 
[here](https://github.com/Shillaker/wasi-libc/blob/master/libc-bottom-half/headers/public/wasm_simd128.h). 

Use of SIMD relies on [WAVM's SIMD support](https://github.com/WAVM/WAVM).

I've hacked about with Eigen on a [fork](https://github.com/Shillaker/eigen-git-mirror) to add WebAssembly
SIMD support. This seems to work but is definitely experimental.

# Downloading
 
The toolchain is packaged as part of the 
[Faasm releases](https://github.com/lsds/faasm/releases), and can be downloaded using 
[the CLI](setup.md):

```bash
# Toolchain - clang, clang++, wasm-ld etc.
inv toolchain.download-toolchain

# Sysroot - wasm-compiled libc, libcxx etc.
inv toolchain.download-sysroot
```

This repo also contains a corresponding [CMake toolchain](../toolchain/WasiToolchain.cmake),
that's used under the hood to build functions and libraries.

# Building

On the rare occasion that we do need to rebuild the toolchain (e.g. when a new 
LLVM version is released), we can run through the steps below.

## Toolchain Updates

When updating the underlying LLVM version of the toolchain you'll need to create a 
[new release](releases.md). The steps should be something like:

- Upgrade the LLVM submodule if necessary (see below)
- Bump up the Faasm version (as outlined in [release docs](releases.md))
- Rebuild the toolchain itself (see below)
- Rebuild the basic sysroot libraries (`inv libs.toolchain --clean`)
- Rebuild 3rd party libraries (Pyodide and Tensorflow, see relevant docs)
- Rebuild and upload _all_ wasm functions (all those under `funcs`)
- Set up the runtime root (see Python docs and Ansible `runtime_fs.yml` playbook)
- Make sure all the tests run
- Create a new [release](releases.md)
- Rebuild all Docker images for this release

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

You can then follow the steps below as normal.

## Building From Scratch

To build from scratch you just need to be in the `toolchain` directory, then run:

```bash
make clean-all
make
```

This a while as it builds a lot of the LLVM toolchain from scratch.

When it finishes, check things are up to the new version with:

```bash
inv toolchain.version
```

### Rebuilding

To rebuild, there different options. The first is just rebuilding libc:

```bash
make clean-libc
make
```

Another is rebuilding all the libs, i.e. libc, libcxx, libcxxabi and compiler-rt:

```bash
make clean-libs
make
```

The final option is to rebuild _everything_, including Clang. This only necessary if you need to change the underlying LLVM or Clang configuration:

```bash
make clean-all
make
```

## Troubleshooting the build

If a given project fails you need to go to the build dir (e.g. for `libcxx` this would 
be `third-party/llvm-project/build/libcxx`) then look at `CMakeFiles/CMakeError.log` and 
`CMakeFiles/CMakeOutput.log`.

One thing to check for is the magic `libclang_rt.builtins-wasm32.tar.gz` file. This should get 
built by the build process but if you need to fiddle with it you can download it as part of the 
[WASI SDK Release](https://github.com/CraneStation/wasi-sdk/releases).
