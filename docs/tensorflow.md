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
rm -rf third-party/tensorflow/tensorflow/lite/tools/make/gen/wasm32-unknown-none_x86_64
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
inv upload tf image --subdir=image
```

Then you can invoke it with:

```
inv invoke tf image
```


