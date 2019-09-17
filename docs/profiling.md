# Profiling

WAVM uses the LLVM JIT libraries to load and execute code at runtime. I've hooked this up with perf events so that
this JITed code will properly be reported, but it requires a special build of LLVM to run.

To set things up you need to do the following:

- Run the `perf.yml` Ansible playbook to set up `perf`
- Create a build of LLVM with perf support by running `./bin/build/llvm_perf` (this takes ages)
- Modify the top-level `CMakeLists.txt` to set `FAASM_CUSTOM_LLVM` to `1`
- Rebuild the target you want to profile

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

Provided the perf notifier is working, this will label functions like `function123`. To map these back to meaningful names you can run the following:

```
./func_sym <user> <function>
```

The output should be at `wasm/<user>/<function>/function.symbols`.

Note that if the perf notifier isn't working, check that the code isn't getting excluded by the pre-processor by looking at the WAVM `LLVMModule.cpp` file.
