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

## Code Generation

To support code generation and AOT WebAssembly with WAMR we need to build
the `wamrc` compiler. This process is outlined 
[in the WAMR README](https://github.com/bytecodealliance/wasm-micro-runtime#build-wamrc-aot-compiler). Note that this requires building LLVM so takes a while.

To continue you must make sure that `wamrc` is now on your path.

Once done, you can compile and generate machine code for Faasm functions as normal, 
passing the `--wamr` flag to the codegen:

```bash
inv compile demo hello
inv codegen demo hello --wamr
```
