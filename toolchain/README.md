# Faasm Toolchain

This is in flux at the moment.

We need a wasm-friendly libcxx and libc with pthreads on. This is a little different to the standard Emscripten/ wasi-sdk approach where threading is turned off.

Unfortunately to do this we need to build a custom LLVM toolchain with a modified musl and libcxx and libcxxabi set to use pthreads.