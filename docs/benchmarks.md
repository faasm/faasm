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
be invoked with your desired number of iterations for native and wasm respectively, e.g. `poly_bench 5 5`.
Results are currently output to `/tmp/polybench.csv`.

## Python

To benchmark CPython execution we use the [Python Performance Benchmark Suite](https://github.com/python/performance).

To set things up you need to look at the relevant Python docs in this directory.

The set of benchmarks can be run with the `py_bench` executable. You can specify a specific benchmark or a single
benchmark, along with the number of wasm and native iterations, e.g. `py_bench all 100 100` or
`py_bench bench_float.py 200 200`. This will output to `/tmp/pybench.csv`.