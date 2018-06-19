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

Once Emscripten is installed, you can compile a simple source file as a stand-alone dynamic library like this:

```
emcc source.c -Os -s WASM=1 -s SIDE_MODULE=1 -o target.wasm
```

Docs on [stand-alone wasm](https://gist.github.com/kripken/59c67556dc03bb6d57052fedef1e61ab).

## WABT

The [WebAssembly binary toolkit](https://github.com/WebAssembly/wabt) is useful for inspecting compiled binaries.

To install and build you need to run:

```
cd /usr/local/code
git clone --recursive https://github.com/WebAssembly/wabt
cd wabt
make
```

You can then run the binaries generated in `/usr/local/code/wabt/out` such as `wasm2wat` which converts `.wasm` files
to text format and prints them.