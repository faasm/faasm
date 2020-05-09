# Debugging and Profiling

## Debugging

As Faasm functions are compiled to WebAssembly and executed using [WAVM](https://github.com/WAVM/WAVM/),
any general tips that apply in WAVM also apply to Faasm.

The instructions below assume that 
- you're building Faasm locally as per the [local dev](local_dev.md) instructions,
- that you built the `func_sym` target,
- that Faasm's built executables are on your `PATH`.

### Symbols

To understand the output of gdb we will need the function symbols available, which we can do with

```
inv disas.symbols <user> <func>
```

This will output the mapping from things like `functionDef123` to the names of functions as they 
appear in the source.

The output should be at `wasm/<user>/<function>/function.symbols`.

### GDB

You can use `gdb` to debug wasm functions with the `simple_runner` CMake target, e.g.

```
# Normally
simple_runner <user> <func>

# GDB
gdb --args simple_runner <user> <func>
```

Because the function itself is loaded with the LLVM JIT libraries, GDB doesn't have the symbols
up front, but if we know the function from the symbols output we can set a breakpoint pending shared
library load.

```
break functionDef1234
```

You can then use normal gdb functionality, albeit with a lack of source information, e.g. view backtraces
inspect stack frames etc.

# Profiling

WAVM uses the LLVM JIT libraries to load and execute code at runtime. This can be connected to perf events 
so that this JITed code will properly be reported, but it requires a special build of LLVM to run.

To set things up you need to do the following:

- Run the `perf.yml` Ansible playbook to set up `perf`
- Create a build of LLVM with perf support by running `./bin/build/llvm_perf` (this takes ages)
- Turn on the `FAASM_PERF_PROFILING` option in you CMake build configuration. (`ccmake <build_dir>` makes this easy).
- Rebuild `codegen_func`, and your runner to build and run the target you want to profile, 

Once this is done you can use perf as described [here](https://lwn.net/Articles/633846/), i.e.:

```
# Do the profiling, e.g.
perf record -k 1 simple_runner <user> <function> 500

# OR
perf record -k 1 poly_bench poly_heat-3d 0 10

# Inject the JIT dumps into
perf inject -i perf.data -j -o perf.data.jitted

# View the report
perf report -i perf.data.jitted
```
 
Note that if the perf notifier isn't working, check that the code isn't getting excluded by the 
pre-processor by looking at the WAVM `LLVMModule.cpp` file.
