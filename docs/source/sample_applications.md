# Sample Applications

Faasm transparently runs C,C++, and Python code cross-compiled to WebAssembly.
However, Faasm does not support all language features and APIs. The limiting
factor is the cross-compilation step: any foreign call (e.g. system calls) in
WASM code needs to be handled by the embedder or host OS.

Over time, Faasm contributors and users have been porting a number of
applications to Faasm. Porting is a two step process. First, the source code
must be cross-compiled to WebAssembly. This happens in different subprojects
depending on the programming language: [faasm/cpp](https://github.com/faasm/cpp)
for C,C++ and [faasm/python](https://github.com/faasm/python) for Python. At
this stage, all foreign calls are left as undefined symbols, and are linked at
runtime in Faasm. Second, the WASM file with undefined symbols must be ran
in Faasm, which means that definitions for all undefined symbols must be
provided.

The [ported_libs](https://github.com/faasm/faasm/tree/main/tests/test/ported_libs)
test directory includes an exhaustive list of the applications that work
out-of-the box (bar some modifications in our fork of the app), and how to run
them. This list includes:
* [LAMMPS](https://github.com/faasm/lammps): a dynamic molecule simulator using
  C++ and MPI.
* [LULESH](https://github.com/faasm/LULESH): a lagrangian shock simulator using
  C++ and OpenMP.
* [FFmpeg](https://github.com/faasm/FFmpeg): libraries for the FFmpeg video-
  processing toolkit in C.
* [ParRes Kernels](https://github.com/faasm/Kernels): set of parallel kernels
  for research using C, MPI and OpenMP.
