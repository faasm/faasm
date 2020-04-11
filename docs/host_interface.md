# Faasm Host Interface

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
 
 ## WASI
 
 To support a broader range of syscalls and compile larger legacy applications
 (such as [CPython](python.md)), Faasm uses a modified 
 [wasi-libc](https://github.com/Shillaker/wasi-libc).
 