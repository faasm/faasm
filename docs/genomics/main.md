# Genomics Use-case

## Native

First you need to install libfaasm natively:

```
inv install-native-tools
```

One that's set up, you can run the following:

```
cd third-party/gem3-mapper
./native_build.sh
```

You can then run the code normally as described in the repo.

## WASM

To build for WASM, you can run:

```
cd third-party/gem3-mapper
./wasm_build.sh
```

