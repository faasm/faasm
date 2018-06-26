## Nodejs

[Install instructions](https://nodejs.org/en/download/package-manager/) contain the following for v10:

```
curl -sL https://deb.nodesource.com/setup_10.x | sudo -E bash -
sudo apt-get install -y nodejs
```

## Emscripten

[Install instructions](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). Basically:

```
cd /usr/local/code
git clone https://github.com/juj/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest

# Run this before executing any emsdk stuff
source /usr/local/code/emsdk/emsdk_env.sh
```

## Compiling

Emscripten can either compile stand-alone wasm or can create an executable JS file which allows integration with the host. The process is documented [here](https://github.com/kripken/emscripten/wiki/WebAssembly-Standalone).

You can either explicitly say you want to compile it as a wasm module (which supports relocating memory and pointers etc.), or you can let the optimiser remove some of the runtime.

Compiling a module:

```
emcc source.c -Os -s WASM=1 -s SIDE_MODULE=1 -o target.wasm
```

Compiling with an optimised JS file:

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
