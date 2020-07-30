# Faasm WAMR Support

Faasm currently provides only provides full support for 
[WAVM](https://github.com/wavm/wavm), but a subset of 
[WAMR](https://github.com/bytecodealliance/wasm-micro-runtime)
support is also in development.

Faasm is independent of the underlying WebAssembly runtime, provided that 
runtime can perform code generation, validation and execution (and be 
embedded in C/C++).

The majority of WAMR-related code can be found in 
[`src/wamr`](../src/wamr).
