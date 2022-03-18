# Threading

Faasm transparently distributes multi-threaded applications across hosts, and
synchronises changes to shared memory across those hosts.

To synchronise shared memory, Faasm tracks which pages of memory have been
written to by remote threads, then merges them back into a single main copy of
the shared address space. Faasm also provides a number of distributed
coordination primitives such as locks and barriers. The logic and messaging
underlying this is implemented in [Faabric](https://github.com/faasm/faabric).

Faasm currently supports two multi-threading APIs: OpenMP and pthreads.

## OpenMP

Faasm supports a number of OpenMP constructs including simple parallel for loops
and reductions. You can find a number of OpenMP example functions in the
[`func/omp` directory](https://github.com/faasm/cpp/tree/main/func/omp) of the
C/C++ repo.

## pthreads

Faasm supports simple creation and joining of pthreads, as well as pthread
mutexes. It also provides stubs for serveral other pthread calls so that
applications can be linked and run without failing (although behaviour of things
like pthread attributes may not be replicated).

To find out more, you can have a look at the following pthreaded functions:

- [Example
  1](https://github.com/faasm/cpp/blob/main/func/demo/threads_check.cpp)
- [Example
  2](https://github.com/faasm/cpp/blob/main/func/demo/threads_local.cpp)
- [Example
  3](https://github.com/faasm/cpp/blob/main/func/demo/threads_memory.cpp)

You can see which pthread calls are supported in
[`src/wavm/threads.cpp`](https://github.com/faasm/faasm/blob/main/src/wavm/threads.cpp).
