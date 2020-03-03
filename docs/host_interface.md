# Faasm Host Interface

To execute functions compiled to WebAssembly, Faasm must define an interface
that allows these functions to interact with the underlying host and runtime.
This interface must support a subset of standard POSIX-like syscalls, as well 
as serverless-specific tasks such as state management, and invoking other 
functions.

## Function invocation and chaining

These cover interactions with serverless functions and their ability to invoke
other functions ("chaining").

| Function | Description  |
|---|---|
| `byte* read_call_input()` | Read input data as byte array | 
| `void write_call_output(out_data)` | Write output data for function |
| `int chain_call(name, args)` | Call function and return `call_id` |
| `int await_call(call_id)` | Await completion of `call_id` |
| `byte* get_call_output(call_id)` | Load the output data of `call_id` |

## State

This section of the host interface covers management of state as outlined in 
[the state docs](state.md).

| Function | Description  |
|---|---|
| `byte* get_state(key, flags)` | Read input data as byte array | 
| `byte* get_state_offset(key, off, flags)` | Write output data for function |
| `void set_state(key, val)` | Write output data for function |
| `void set_state_offset(key, val, len, off)` | Write output data for function |
| `void push/pull_state(key)` | Write output data for function |
| `void push/pull_state_offset(key, off)` | Write output data for function |
| `void append_state(key, val)` | Write output data for function |
| `void lock_state_read/write(key)` | Write output data for function |
| `void lock_state_global_read/write(key)` | Write output data for function |
 
## POSIX-like system calls

This section covers "standard" POSIX-like system calls, some of which are also found 
in [WASI](https://wasi.dev/).

| Category | Function | Description  |
|---|---|---|
| Dynamic linking | `void *dlopen/dlsym(...)` | Dynamic linking of libraries | 
| Dynamic linking | `int dlclose(...)` | As above |
| Memory | `void* mmap(...), int munmap(...)` | Memory grow/shrink only |
| Memory | `int brk(...), void* sbrk(...)` | Memory grow/shrink only |
| Network | `int socket/connect/bind(...)` | Client-side networking only |
| Network | `size_t send/recv(...)` | Send/recv via virtual interface |
| File I/O | `int open/close/dup/stat(...)` | Per-user virtual filesystem access |
| File I/O | `size_t read/write(...)` | As above |
| Timing | `int gettime(...)` | Per-user monotonic clock only |
| Random | `size_t getrandom(...)` | Uses underlying `host/dev/urandom` |
 
 ## Further extensions
 
 The Faasm host interface may grow over time to support a wider range of POSIX-like
 system calls, and other functionality appropriate to a serverless context. Support for
 collective communication over MPI is an example of this, and outlined in the 
 [Faasm MPI docs](mpi.md).
  