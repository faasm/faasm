# Faasm and Threading

Serverless applications benefit most from the parallelism of many small functions
distributed across hosts. This makes it hard to port monolithic applications that 
rely on shared memory and multi-threading.

To support these kinds of applications, Faasm includes implementations of pthreads 
and a subset of [OpenMP](openmp.md). 

Under the hood, Faasm converts threading operations to serverless-specific ones. 
For example, spawning a thread may result in the creation of a new function invocation 
on another host. 

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

Faasm handles migration across hosts using [proto-functions](proto_faaslets.md), which 
duplicate a function's state on another host. This means that the memory a thread sees on 
its new host is the same as on the original, however, writes obviously don't persist. 

## Shared data

The main challenge in distributing threads in a serverless context is handling writes on
shared data. These must be handled with using Faasm's [shared state](state.md) which,
with minimal changes, lets the code work normally.

An example of a distributed threaded application can be found [in the examples](../func/demo/threads_dist.cpp).

## WIP

_This is a work in progress, more docs to come._