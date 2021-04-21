# Faasm OpenMP Runtime

Faasm supports OpenMP by intercepting calls to the OpenMP `libomp` runtime with
the `libfaasmp` library. It can run OpenMP threads in two different modes (which
will be determined via the Faabric scheduler).

- Intra-Faaslet parallelism, by using WebAssembly threads inside the Faaslet
as OpenMP thread executor. 

- Inter-faaslet parallelism, by creating other Faaslet as executor for the
and scheduling them in a distributed fashion.
