# Faasm OpenMP Support

Faasm supports OpenMP by implementing the OpenMP runtime functions as part of the host
interface (see below for details).

## OpenMP functions

To compile, upload and run the prebuilt OpenMP demo function:

```
inv compile omp hellomp
inv upload omp hellomp
inv invoke omp hellomp
```

## Adding support for new OpenMP runtime functions

Runtime OMP functions are implemented just like any other host interface function 
and can be found in `src/wasm/syscalls_openmp.cpp`. 

The list of acceptable unresolved imports is in `lib-faasmp/faasmp.imports`.

To add a new function you need to include it in both of these files. 

## Native functions

To study the behaviour of OpenMP code, it is possible to run it locally after applying
the local dev set-up as follows
```
inv compile omp <native function> --local-omp
inv native-run <native function> [-ltrace|-strace]
```

Below is the support state of the functions. All functions can be found in the
[native function directory](func/omp/native-functions/). The `intel-` prefix indicates
they are coming from [Intel kernel](https://github.com/ParRes/Kernels/tree/master/OPENMP).

| Function                 | Native support | Faasm support |
|--------------------------|----------------|---------------|
| `create-parallel.cpp`    |        y       |               |
| `create-teams.cpp`       |        y       |               |
| `intel-nstream.c`        |        y       |               |
| `intel-synch-p2p.c`      |                |               |
| `intel-transpose.c`      |        y       |               |
| `parallel-for.cpp`       |        y       |               |
| `reduction-average.cpp`  |        y       |               |
| `reduction-integral.cpp` |        y       |               |

