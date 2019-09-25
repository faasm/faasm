# Tensorflow Support

We can build Tensorflow Lite to WebAssembly using the standard Faasm toolchain.

To do this, make sure you've updated all the git submodules, then you can run:

```
source workon.sh
inv compile-tf-lite
```

The build output ends up at `third-party/tensorflow/tensorflow/lite/tools/make/gen`.

## Usage

See the example Tensorflow function using this build at `func/tf/image/label_image.cc`.