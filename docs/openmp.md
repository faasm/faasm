# Faasm OpenMP Support

To upload and run the prebuilt OpenMP demo function:

```
inv upload omp hellomp --prebuilt
inv invoke omp hellomp
```

## Compiling functions

Note that this requires either the Dockerised toolchain as described in the README or 
a local dev set-up as described in `local_dev.md`.

The easiest way to add new functions is to add them to the `func/omp` directory alongside
`hellomp.cpp` and add them to `CMakeLists.txt` accordingly. You can then use:

```
inv compile omp <your function>
inv upload omp <your function>
inv invoke omp <your function>
```

If you don't want to keep running and restarting Faasm when you're developing, you can 
execute the function using the `simple_runner` target in this project (takes a user 
and function as arguments but must be built as you'd build any other CMake target).

## Adding support for new OpenMP runtime functions

Runtime OMP functions are implemented just like any other host interface function 
and can be found in `src/wasm/syscalls_openmp.cpp`. 

The list of acceptable unresolved imports is in `lib-faasmp/faasmp.imports`.

To add a new function you need to include it in both of these files. 

To recompile the new libraries you can use: 
```
inv compile-libfaasm
```
