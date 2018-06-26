# Faasm with WAVM

## Set-up

### WAVM

[WAVM](https://github.com/AndrewScheidecker/WAVM) is a stand-alone wasm virtual machine. It executes only wasm and knows nothing about Javascript.

To import into CLion you need to install `clang-6.0` (see notes in Research), then just import the project normally.

To avoid having all the debug output, you can create a "Release" CMake configuration (via Settings -> Frameworks etc. -> CMake). This will then be available in the builds drop-down in the top right.The debug output is still probably worth a look though.

You can then run the "build all" target which should produce various output under the `bin` output directory. In here will be `wavm` which can be used to execute WebAssembly files.

### Binaryen

To compile files we need to use [Binaryen](https://github.com/WebAssembly/binaryen). Clone the repo then use:

```
cd /usr/local/code/binaryen
cmake . && make
```

### WABT

The [WebAssembly binary toolkit](https://github.com/WebAssembly/wabt) is useful for inspecting compiled binaries.

To install and build you need to run:

```
cd /usr/local/code
git clone --recursive https://github.com/WebAssembly/wabt
cd wabt
make
```

To make things easier you can add `/usr/local/code/wabt/out/` to your path, then do things like:

```
wasm2wat my_wasm_file.wasm
```

## Compilation

Compiling source to WASM does:

- `clang -emit-llvm` to get a `.ll` file
- `llc` to compile the `.ll` file to a `.s` file
- `s2wasm` to convert the `.s` file to a `.wat` file (wasm text repr)
- `wat2wasm` to convert this to the final `.wasm` binary

We can then run `wasm-opt` on the resulting file to optimise it.

## Old Notes

### Emscripten

```
cd /usr/local/code
git clone https://github.com/juj/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest

# Run this before executing any emsdk stuff
source /usr/local/code/emsdk/emsdk_env.sh
```

You can then compile normally but ignore the generate JS file:

```
emcc source.c -Os -s WASM=1
```

