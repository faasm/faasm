# Host interface

Faasm functions can interact with the Faasm runtime and underlying host
through the host interface. This lets them do things like manage state and
interact with other functions.

The host interface includes:

- Function input/ output
- Chaining functions
- State management
- [WASI](https://wasi.dev/)
- Dynamic linking
- [pthreads](threads.md)
- [MPI](mpi.md)
- [OpenMP](openmp.md)

## Function input/output and chaining

These cover interactions with serverless functions and their ability to invoke
other functions ("chaining").

| Function | Description  |
|---|---|
| `byte* read_call_input()` | Read input data as byte array |
| `void write_call_output(out_data)` | Write output data for function |
| `int chain_name(name, args)` | Call function by name and return `call_id` |
| `int chain_ptr(ptr, args)` | Call function pointer and return `call_id` |
| `int await_call(call_id)` | Await completion of `call_id` |
| `byte* await_call_output(call_id)` | Await completion and get output of `call_id` |

## State

This section of the host interface covers management of state as outlined in
[the state docs](state.md).

| Function | Description  |
|---|---|
| `byte* get_state(key, flags)` | Get pointer to state value for `key` |
| `byte* get_state_offset(key, off, flags)` | Get pointer to state value for `key` at an offset |
| `void set_state(key, val)` | Set state value for `key` |
| `void set_state_offset(key, val, len, off)` | Set `len` bytes of state value at offset for `key` |
| `void push/pull_state(key)` | Push/pull global state value for `key` |
| `void push/pull_state_offset(key, off)` | Push/pull global state value for `key` at offset |
| `void append_state(key, val)` | Append data to state value for `key` |
| `void lock_state_read/write(key)` | Lock local copy of state value for `key` |

 ## POSIX-like calls and WASI

 Faasm supports WASI, but adds and customises further POSIX-like system calls
 to support a broader range of legacy applications and libraries (such as
 [CPython](python.md)).

 This is done in part with a modified [wasi-libc](https://github.com/faasm/wasi-libc).

 The main additions and modifications are as follows:

 | Category | Function | Description  |
 |---|---|---|
 | Dynamic linking | `void *dlopen/dlsym(...)` | Standard POSIX-like dynamic linking |
 | Dynamic linking | `int dlclose(...)` | As above |
 | Memory | `void* mmap(...), int munmap(...)` | Only permits memory growth and file mapping |
 | File I/O | `int dup(...)` | Necessary for certain legacy applications |

 Faasm supports standard filesystem syscalls, but provides a serverless-specific implementation.
 Because functions may be executing on different hosts, all filesystem accesses are
 _global-read-local-write_, i.e. functions can access files from any host, but writes are only
 made locally. This is sufficient to support legacy filesystem use-cases, while
 any data sharing between functions can be done through [state](state.md).

 As mentioned above, Faasm also supports [pthreads](threads.md), which also introduce a number
 of extra POSIX syscalls.
