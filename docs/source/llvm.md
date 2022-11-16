# LLVM use in Faasm

LLVM is a key dependency in Faasm. At the time of the writing, the Faasm
ecosystem uses LLVM 13.

LLVM is used all across the project:
* In [faasm/cpp](https://github.com/faasm/cpp), LLVM is used to cross-compile
  C and C++ code to WebAssembly. Different LLVM versions ship with different
  WASM backends, which may alter the cross-compilation step.
* All across the project, we use `clang++` to build C++ code.
* In particular, in [faasm/faasm](https://github.com/faasm/faasm), we build
  different WASM runtimes, namely [WAVM](https://github.com/faasm/WAVM) and
  [WAMR](https://github.com/faasm/wasm-micro-runtime). WASM runtimes do several
  optimizations to WASM code during code generation, they therefore make heavy
  use of the LLVM libraries.

We could _potentially_ use different versions of LLVM across the project. In
the past, LLVM 10 and LLVM 13 have co-existed in the project: the former to
cross-compile to WebAssembly, and the latter to compile C++ code. This is
why we install them differently: the LLVM toolkit to build C++ code is
installed in [faasm/faabric](
https://github.com/faasm/faabric/blob/main/docker/faabric-base.dockerfile#L4-L57)
from APT, whereas the LLVM toolkit to cross-compile to WebAssembly is build from
[our LLVM fork](https://github.com/faasm/llvm-project) in [faasm/cpp](
https://github.com/faasm/cpp/blob/main/docker/llvm.dockerfile). It is preferable
to maintain the same version throughout, but the former is easier to update than
the latter, so they can have different upgrade life-cycles.

## Upgrading LLVM

Given the above, upgrading the LLVM version is a delicate procedure, and things
are likely to break.

To upgrade the LLVM version we use to build C++ code, you need to update
the [`faabric-base` dockerfile](
https://github.com/faasm/faabric/blob/main/docker/faabric-base.dockerfile#L4-L57)
and propagate the changes. For this you will need to:
* faasm/faabric: bump the code version `inv git.bump`
* faasm/faabric: modify `faabric-base` and re-build `inv docker.build -c faabric-base --push --nocache`
* faasm/faabric: update the `faabric-base` version tag in [`faabric.dockerfile`]()
* faasm/faabric: tag the code (GHA will rebuild the image) `inv git.tag`
* faasm/faabric: re-run the tests once containers are built
* faasm/cpp: bump the code version `inv git.bump`
* faasm/cpp: update the `faabric-base` version tag in [`cpp-sysroot.dockerfile`](
https://github.com/faasm/cpp/blob/main/docker/cpp-sysroot.dockerfile#L4)
* faasm/cpp: tag the code `inv git.tag`
* faasm/cpp: re-run the tests once containers are built
* faasm/python: bump the code version `inv git.bump`
* faasm/python: update the `cpp-sysroot` tag in [`Dockerfile`](
https://github.com/faasm/python/blob/main/Dockerfile#L1)
* faasm/python: tag the code `inv git.tag`
* faasm/python: re-run the tests once containers are built
* faasm/faasm: bump the code version `inv git.bump`
* faasm/faasm: bump the `faabric-base` version in [`cpp-root.dockerfile`](
https://github.com/faasm/faasm/blob/main/docker/cpp-root.dockerfile#L1)
* faasm/faasm: bump the `cpython` and `cpp-root` version in [`base.dockerfile`](
https://github.com/faasm/faasm/blob/main/docker/base.dockerfile#L2-L5)
* faasm/faasm: bump the `clients/cpp`, `clients/python`, and `faabric`
  submodules to include the latest changes (remember to do this step again
  after the branches have been merged in, as we want the submodules to point
  to the `main` branch)
* faasm/faasm: tag the code `inv git.tag`
* faasm/faasm: re-run the tests once containers are built

To upgrade the LLVM version that we use to cross-compile to WASM, you will
have to:
* faasm/cpp: follow the [instructions](
https://github.com/faasm/cpp/blob/main/docs/upgrade-llvm.md) on how to upgrade
LLVM there
* Repeat the steps in the previous point from `faasm/cpp` onwards.
