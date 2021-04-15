# Profiling

Note that although using profilers inside containers is possible, it's best to
run the on Faasm binaries built and executed outside a container. 

See the [dev docs](development.md) on how to set this up.

## Set-up

- Build Faasm outside a container, as described in the [dev docs](development.md)
- Install BCC tools
- Install `perf`
- Build LLVM with JIT perf support (see below)

The Ubuntu BCC binaries are quite out of date but should be sufficient. If not
you can look at their
[docs](https://github.com/iovisor/bcc/blob/master/INSTALL.md):

```
sudo apt-get install bpfcc-tools linux-headers-$(uname -r) 
```

Set up `perf` to run without `sudo`:

```
sudo sysctl -w kernel.perf_event_paranoid=-1
sudo sysctl -w kernel.kptr_restrict=0
```

## Building LLVM with JIT perf support

You can use perf with a standard Faasm build, but this may have large gaps with
`perf.<PID>.map`. This is because WAVM uses the LLVM JIT libraries to load and 
execute code at runtime, including any WebAssembly.

Unfortunately the default build of the LLVM JIT libraries doesn't include the
perf events that we need, so we need to rebuild LLVM with the right flags (takes
a while):

```bash
./bin/build_llvm_perf.sh
```

Now you can rebuild the parts of Faasm you're profiling, e.g.

```bash
# Note the --perf here to switch on the build against the custom LLVM
inv dev.cmake --perf --clean

inv dev.cc simple_runner
```

Note that this will also set `-fno-omit-frame-pointer` to the compile flags
which can also improve the level of detail exposed in the perf data generally.

WebAssembly functions will be output with names like `functionDef123`, see the
[development docs](development.md) on how to map these back to names in the
source (using `inv disas`).

Once this is done you can use `perf` with JIT symbols as described
[here](https://lwn.net/Articles/633846/).

## perf

```
# Standard CPU profiling of 1000 runs of demo/hello
perf record -k 1 -F 99 -g simple_runner demo hello 1000

# Inject the JIT dumps into perf data
perf inject -i perf.data -j -o perf.data.jit

# View the report
perf report -i perf.data.jit
```
 
Note that if the `perf` notifier isn't working, check that the code isn't
getting excluded by the pre-processor by looking at the WAVM `LLVMModule.cpp`
file and grepping for `WAVM_PERF_EVENTS`.

You can also check the
[diff](https://github.com/WAVM/WAVM/compare/master...faasm:faasm) of the Faasm
WAVM fork to see the changes that were made.

## Flame graphs

There is a task in the Faasm CLI for creating 
[Flame graphs](https://github.com/brendangregg/FlameGraph) which automatically 
include the disassembled WebAssembly function names.

Note that this requires the custom LLVM build described above.

```
# Make sure you can run and disassemble the functions
inv dev.cc simple_runner
inv dev.cc func_sym

# Run the flame graph task (which will run perf, replace symbols etc.)
inv flame demo echo --reps=5000 --data="foobar"

# Open the flame graph in your browser
firefox flame.svg
```

You can use the search feature in the flame graph to find things related to wasm
by searching (Ctrl+F) for `wasm`.

If you want to do custom set-up of a specific function, you can write an adapted
version of the `simple_runner`, to run your function, then pass it in as a 
command to `inv flame`:

```
inv dev.cc my_runner

inv flame <user> <func> --cmd="my_runner <args>"

firefox flame.svg
```

## Valgrind

WAVM seems to require a large stack, so you'll have to bump up the
`main-stacksize`. An example run with Callgrind might look like:

```bash
valgrind --tool=callgrind --main-stacksize=16777216 simple_runner demo hello
```

Unfortunately we've not yet worked out a way to get valgrind to include the
JITed wasm symbols, so at the moment it's only useful for profiling the Faasm
runtime itself.
