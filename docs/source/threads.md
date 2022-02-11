# Threading

Serverless applications benefit most from the parallelism of many small
functions distributed across hosts. Spawning one or two threads to do background
work still makes sense, but using threading and shared memory to parallelise a
larger application is not necessarily effective in a serverless context.

To support larger multi-threaded applications, Faasm can transparently
distribute threads across multiple serverless functions, thus allowing them to
take advantage of serverless parallelism. 

This functionality is accessed in C/C++ through pthreads and
[OpenMP](openmp.md).  Faasm intercepts normal threading calls and chains a new
serverless function to execute the thread, potentially on another host. Shared
data is managed with [shared state](state.md) and the process memory is migrated
across hosts with [Proto-Faaslets](proto_faaslets.md).

## pthreads
 
Faasm supports the following pthread functions. Although all `pthread_attr_XXX`
calls are supported, the attributes themselves may be ignored in a Faasm
context.
 
| Function | Description  |
|---|---|
| `int pthread_create(...)` | Spawn a new thread | 
| `int pthread_join(...)` | Await thread completion |
| `void pthread_exit(...)` | Exit the current thread |
| `void pthread_attr_XXX` | All attr-related calls |

## OpenMP

Faasm OpenMP support has [separate docs](openmp.md).

