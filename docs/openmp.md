# Faasm OpenMP Runtime

Faasm supports OpenMP by replacing the LLVM OpenMP `libomp` runtime with the
`libfaasmp` library. It can run OpenMP threads in two different modes:
* intra-faaslet parallelism, by using WebAssembly threads inside the faaslet
as OpenMP thread executor. The local threads are managed by a thread pool
which size (`OMP_THREAD_POOL_SIZE` env var) is 0 by default
* inter-faaslet parallelism, by creating other faaslet as executor for the
and scheduling them in a distributed fashion.

## OpenMP functions

To compile, upload and run the prebuilt OpenMP demo function:

```
inv compile omp hellomp
inv upload omp hellomp
inv invoke omp hellomp
```

## Adding support for new OpenMP runtime functions

Runtime OMP functions are implemented just like any other host interface function 
and can be found in `src/wasm/openmp.cpp`. 

The list of acceptable unresolved imports is in `libs/faasmp/faasmp.imports`.

To add a new function you need to include it in both of these files.

LLVM adds GNU compatibility data symbols on some features for synchronisation.
The Wasm linker needs to be aware of the sizes and the names of the symbols,
which are not valid C/C++ identifiers, and as such are resolved in assembly in
`libs/faasmp/z_Linux_asm.S`. An error from the linker when compiling new parts
of the API implies you need to you need to add the symbol to this file.
