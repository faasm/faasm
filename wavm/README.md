# Faasm with WAVM

[WAVM](https://github.com/AndrewScheidecker/WAVM) is a stand-alone wasm virtual machine. It executes only wasm and knows nothing about Javascript.

To import into CLion you need to install `clang-6.0` (see notes in Research), then just import the project normally.

You can then run the "build all" target which should produce various output under the `bin` output directory. In here will be `wavm` which can be used to execute WebAssembly files.

## Compilation

WAVM takes files compiled by Emscripten so the usual compilation steps apply. [Install instructions](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html) are basically:

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

## WABT

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

