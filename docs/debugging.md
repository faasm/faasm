# Debugging and Profiling

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

We can use `gdb` normally on the `simple_runner` target, e.g.

```
inv dev.cc simple_runner
gdb simple_runner

break functionDef123

run <user> <func>
```

Note that because the function itself is loaded using LLVM JIT libraries, GDB 
doesn't have the symbols up front, but we can still set breakpoints pending a
shared library load.

## Perf

Perf (and Valgrind) requires administrative privileges that are hard to
replicate in a container (although not impossible). The easiest way to use these
tools is to build a Faasm executable outside of a container. See the [dev
docs](development.md) for details on how to do this.

WAVM uses the LLVM JIT libraries to load and execute code at runtime. This can
be connected to perf events so that this JITed code will properly be reported,
but it requires a special build of LLVM to run.

To set things up you need to do the following:

- Run the [`perf.yml`](../ansible/perf.yml) Ansible playbook to set up `perf`
- Create a build of LLVM with perf support by running `./bin/build_llvm_perf.sh`
  (this takes a while depending on your machine)

Now you can rebuild the parts of Faasm you're profiling, e.g.

```bash
# Note the --perf here to switch on the build against the custom LLVM
inv dev.cmake --perf --clean

inv dev.cc simple_runner
```

Once this is done you can use perf as described
[here](https://lwn.net/Articles/633846/), e.g.:

```
# Do the profiling of 500 runs of demo/hello
perf record -k 1 simple_runner demo hello 500

# Inject the JIT dumps into perf data
perf inject -i perf.data -j -o perf.data.jitted

# View the report
perf report -i perf.data.jitted
```
 
Note that if the perf notifier isn't working, check that the code isn't getting
excluded by the pre-processor by looking at the WAVM `LLVMModule.cpp` file and
grepping for `WAVM_PERF_EVENTS`.

You can also check the
[diff](https://github.com/WAVM/WAVM/compare/master...faasm:faasm) of the Faasm
WAVM fork.

