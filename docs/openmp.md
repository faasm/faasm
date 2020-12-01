# Faasm OpenMP Runtime

Faasm supports OpenMP by replacing the LLVM OpenMP `libomp` runtime with the
`libfaasmp` library. It can run OpenMP threads in two different modes:

- Intra-Faaslet parallelism, by using WebAssembly threads inside the Faaslet
as OpenMP thread executor. The local threads are managed by a thread pool
which size (`OMP_THREAD_POOL_SIZE` env var) is 0 by default.

- Inter-faaslet parallelism, by creating other Faaslet as executor for the
and scheduling them in a distributed fashion.

## OpenMP functions

To compile, upload and run the pre-built OpenMP demo function:

```
inv compile omp hellomp
inv upload omp hellomp
inv invoke omp hellomp
```

## Adding support for new OpenMP runtime functions

Runtime OMP functions are implemented just like any other host interface
function and can be found in `src/wasm/openmp.cpp`. 

The definition of Faasm's OpenMP interface can be found in the [`libfaasmp`
directory](https://github.com/faasm/faasm-toolchain/tree/master/libfaasmp) of
the `faasm-toolchain` repo.

To add a new function you need to include it in both the `libfaasmp` definition,
and the Faasm host interface.

LLVM adds GNU compatibility data symbols on some features for synchronisation.
The wasm linker needs to be aware of the sizes and the names of the symbols,
which are not valid C/C++ identifiers, and as such are resolved in assembly in
`libfaasmp/z_Linux_asm.S` in the `faasm-toolchain` repo. An error from the
linker when compiling new parts of the API implies you need to you need to add
the symbol to this file.
