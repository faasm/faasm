# Tensorflow Support

We can build Tensorflow Lite to WebAssembly using the standard Faasm toolchain.

To do this, make sure you've updated all the git submodules, then you can run:

```
source workon.sh
inv compile-tflite
```

The build output ends up at `third-party/tensorflow/tensorflow/lite/tools/make/gen`.

If you need to rebuild the wasm part:

```
inv compile-tflite --clean
```

## Usage

See the example Tensorflow function at `func/tf/image.cc`.

Executing this function requires some state data (the model) and some shared files (the image) to be available, so you can run:

```
inv tf-upload-data tf-state-upload
```

You can then upload the prebuilt function with:

```
inv upload tf image --prebuilt
```

Or compile and upload yourself (requires TFLite to be built locally):

```
inv compile --func=image
inv upload tf image
```

Then you can invoke it with:

```
inv invoke tf image
```

## Eigen Fork

To support WASM simd instructions I've hacked about with Eigen on a [fork](https://github.com/Shillaker/eigen-git-mirror). It _seems_ to work but isn't well tested. 

TFLite will be compiled against the version of Eigen downloaded as part of its 3rd party deps, so if you need to change it and rebuild you'll need to run:

```
cd third-party/tensorflow/tensorflow/lite/tools/make
rm -r downloads/eigen/
./download_dependencies.sh
```

