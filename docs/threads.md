# Faasm and Threading

Serverless applications benefit most from the parallelism of many small functions
distributed across hosts. This makes the role of threads unclear, and makes it hard 
to port monolithic applications that rely on shared memory and multi-threading.

To support these kinds of applications, Faasm provides mechanisms to transparently
distribute threaded applications across multiple hosts. These are accessed through 
subsets of pthreads and [OpenMP](openmp.md). 

Under the hood, Faasm converts threads into new serverless functions, handling 
shared data through [shared state](state.md) and duplicating memory through 
[proto-functions](proto_faaslets.md).

## pthreads
 
Faasm supports the following pthread functions. Although all `pthread_attr_XXX` calls
are supported, the attributes themselves may be ignored in a Faasm context.
 
| Function | Description  |
|---|---|
| `int pthread_create(...)` | Spawn a new thread | 
| `int pthread_join(...)` | Await thread completion |
| `void pthread_exit(...)` | Exit the current thread |
| `void pthread_attr_XXX` | All attr-related calls |

## Migrating threads across hosts

Threads are migrated across hosts using a version of [proto-functions](proto_faaslets.md), 
which duplicate a function's memory and execution state on another host. While the thread 
initially sees the same memory as its parent, writes to this memory are not propagated 
across hosts. Writes to shared data are handled with Faasm's [shared state](state.md).

An example of a distributed threaded application can be found [in the examples](../func/demo/threads_dist.cpp).

## Wasm threads

Faasm doesn't yet support the [Wasm threading proposal](https://github.com/WebAssembly/threads), 
but its implementation would look a lot like the support for pthreads. 

## WIP

_This is a work in progress, more docs to come._