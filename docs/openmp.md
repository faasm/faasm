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
and can be found in `src/wasm/openmp.cpp`. 

The list of acceptable unresolved imports is in `lib-faasmp/faasmp.imports`.

To add a new function you need to include it in both of these files. 
