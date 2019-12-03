# Tensorflow Support

We can build Tensorflow Lite to WebAssembly using the standard Faasm toolchain.

To do this, make sure you've updated all the git submodules, then you can run:

```
source workon.sh
inv compile-tf-lite
```

The build output ends up at `third-party/tensorflow/tensorflow/lite/tools/make/gen`.

If you need to rebuild the wasm part:

```
inv compile-tf-lite --clean
```

## Usage

See the example Tensorflow function at `func/tf/image/label_image.cc`.

This requires a number of shared files to be available, so once you have an environment running (e.g. with docker-compose), you can run:

```
inv tf-upload-data
```

You can use the prebuilt function with:

```
inv upload tf image --prebuilt
```

Or compile and upload yourself:

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
