# Profiling

WAVM uses the LLVM JIT libraries to load and execute code at runtime. I've hooked this up with perf events so that
this JITed code will properly be reported, but it requires a special build of LLVM to run.

To set things up you need to do the following:

- Run the `perf.yml` Ansible playbook to set up `perf`
- Create a build of LLVM with perf support by running `./bin/build/llvm_perf`
- Modify the top-level `CMakeLists.txt` to set both `FAASM_PROFILE` and `FAASM_CUSTOM_LLVM` to `1`
- Rebuild the whole project

Once this is done you can use perf as described [here](https://lwn.net/Articles/633846/), i.e.:

```
# Do the profiling
perf record -k 1 func_runner <user> <function>

# Inject the JIT dumps into
perf inject -i perf.data -j -o perf.data.jitted

# View the report
perf report -i perf.data.jitted
```