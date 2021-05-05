# Debugging

Code in Faasm has been cross-compiled to WebAssembly, so we lose a lot of
debugging information. However, there are still some options for chasing down
issues.

## Backtrace

If you're seeing a failure nested somewhere deep in an application but can't
work out how it's getting there, you can use the `__faasm_backtrace` function
to tell the runtime to print the stacktrace at any point in your code.

This can be very useful for debugging issues with Python C-extensions.

If you can locate the line where the problem might occur, e.g.

```c
void someDeeplyNestedFunction() {
    // The line that you know has a problem
}
```

You can change this to:

```c
extern "C" {
#include <faasm/host_interface.h>
}

void someDeeplyNestedFunction() {

    __faasm_backtrace(0);

    // The line that you know has a problem
}
```

Note that the line numbers in the stacktrace won't match up to the original
source, so you'll just need to grep for it.

## WAVM function symbols

WAVM assigns certain functions names like `functionDef123`, and we need to map
these back to the original source to understand the output of a debugger.

To do this:

```
inv dev.cc func_sym
inv disas.symbols <user> <func>
```

This will output a mapping from names like `functionDef123` to the names of
functions as they appear in the source.

The output will appear at
`${FAASM_LOCAL_DIR}/wasm/<user>/<function>/function.symbols`.

## GDB

We can use `gdb` normally on the `func_runner` target, e.g.

```
inv dev.cc func_runner
gdb func_runner

break functionDef123

run <user> <func>
```

Note that because the function itself is loaded using LLVM JIT libraries, GDB
doesn't have the symbols up front, but we can still set breakpoints pending a
shared library load.

