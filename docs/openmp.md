# Faasm OpenMP Runtime

Faasm supports OpenMP by intercepting calls to the OpenMP `libomp` runtime with
the `libfaasmp` library. It can run OpenMP threads in two different modes:

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

