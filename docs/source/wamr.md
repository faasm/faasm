# Integration with WAMR

Faasm supports [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime) as
a first class WASM runtime, both for regular `x86_64` and `SGX` execution.

We [embed](https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/embed_wamr.md)
WAMR, meaning we compile it as a library and link against it. There are some
caveats to keep in mind when using WAMR.

First, our use case defers slightly from the envisioned embedding scenario.
There, the same thread of execution intialises the runtime, loads the module,
instantiates the module, creates an execution environment, and calls the WASM
function. However, Faasm is multi-threaded, and different Faaslets, executing
different WASM modules, will share the same WAMR runtime instance. This
instance has some global state that we need to protect with a mutex.

In addition, whenever the thread initialising the runtime, and the thread using
it differ we must initialise (and clean-up) the thread environment **before**
creating an execution environment.

Second, the different abstractions in WAMR are clearly depicted in their
[documentation](https://bytecodealliance.github.io/wamr.dev/blog/the-wamr-memory-model/).
Most notably, execution environments are not thread-safe, and they are bound to
the execution of one WASM module instance.

Third, WAMR is written in C. In a regular Faasm execution WAMR's stack frames
will sit between Faasm's C++ frames. Upper frames in Faasm may throw exceptions
to communicate with bottom frames. These are lost in WAMR. As a consequence
we use a `setjmp,longjmp` combination.

WAMR considers a variety of threading models for threads sharing the same
module instance. We will explore them as we introduce shared memory support for
WAMR.
