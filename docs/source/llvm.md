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

The best advice is to replicate the steps followed in another PR to upgrade
LLVM. See [here for the PR that upgraded to LLVM 13](faasm/faasm#705).
