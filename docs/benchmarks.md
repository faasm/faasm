# Benchmarks

## Polybench/C

To test pure computation against the native environment we can use the
[Polybench/C benchmark](http://web.cse.ohio-state.edu/~pouchet.2/software/polybench/).

The code is checked into this repository and can be compiled to wasm and uploaded as follows:

```
# Compile to wasm
inv compile --user=polybench --clean

# Upload (must have an upload server running)
inv upload-all
```

We can compile the same functions natively as follows:

```
./bin/build_polybench_native.sh
```

The `poly_bench` executable will then run a comparison of the wasm and native versions. This must
be invoked with your desired number of iterations for native and wasm respectively, e.g.

```
poly_bench all 5 5
```

Results are currently output to `/tmp/polybench.csv`.

_Note - we had to leave out the BLAS benchmarks as BLAS is not supported in Faasm_.

## Python

To benchmark CPython execution we use the [Python Performance Benchmark Suite](https://github.com/python/performance).

All python code runs in the same function which can be set up according to the python docs in this repo.

The set of benchmarks can be run with the `py_bench` executable. You can specify a specific benchmark or a single
benchmark, along with the number of wasm and native iterations, e.g. `py_bench all 100 100` or
`py_bench bench_float.py 200 200`. This will output to `/tmp/pybench.csv`.

Each benchmark requires porting the required dependencies, so some were unfeasible and other were too much work:

- `chameleon` - too many deps
- `django_template` - pulls in too many dependencies
- `hg_startup` - runs a shell command
- `html5lib` - dependencies (might be fine)
- `pathlib` - requires more access to the filesystem that we support
- `python_startup` - runs a shell command
- `regex_compile` - needs to import several other local modules (should be possible, just fiddly)
- SQL-related - SQLAlchemy not worth porting for now. SQLite also not supported but could be
- `sympy` - sympy module not yet ported but could be
- `tornado` - Tornado not ported (and don't plan to)


## Profiling

### Profiling Polybench/C

To profile the native version of the code, you need to run `./bin/build_polybench_native.sh Debug`

Then you can directly run the native binary:

```
perf record -k 1 ./func/build_native/polybench/poly_ludcmp
mv perf.data perf.data.native
perf report -i perf.data.native
```

Provided you have set up the wasm profiling set-up as described in the profiling docs, you can do something
similar:

```
perf record -k 1 poly_bench poly_ludcmp 0 5
perf inject -i perf.data -j -o perf.data.wasm
perf report -i perf.data.wasm
```

Note that for wasm code the output of the perf reports will be funciton names like `functionDef123`.
To generate the mapping of these names to the actual functions you can run the `func_sym` executable
e.g.

```
func_sym polybench 3mm
```