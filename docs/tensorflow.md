# Tensorflow Lite in Faasm

[Tensorflow Lite](https://www.tensorflow.org/lite/) is well suited to running 
inference tasks in a resource-constrained serverless context. 
 
It is written in C/C++ hence we can build Tensorflow Lite to WebAssembly using the 
standard Faasm toolchain.  

Faasm currently only supports the C/C++ API, but building the Python API using 
Faasm's Python support should be possible and on the to-do list. 

## Compiling TensorFlow Lite to WebAssembly

To do this, make sure you've checked out this project and updated all the git submodules, 
then, from the Faasm CLI:

```bash
inv libs.tflite
```

The build output ends up at `third-party/tensorflow/tensorflow/lite/tools/make/gen`.

## Building a C/C++ function with TF Lite

A function implementing image classification is included 
[in the examples](../func/tf/image.cc). This is based on the 
[example in the TF Lite repo](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/examples/label_image). 
The data and model for the example are stored in [this repo](../func/tf/data).

To run the example function, you need to run a local Faasm cluster (as described in the [README](../README.md)), 
then:

```bash
# Upload files and state
inv tf.upload tf.state

# Upload the function (takes a few seconds)
inv upload tf image

# Invoke
inv invoke tf image
```

## Eigen Fork

To support WASM simd instructions I've hacked about with Eigen on a 
[fork](https://github.com/Shillaker/eigen-git-mirror). It _seems_ to work but isn't well 
tested. 

TF Lite will be compiled against the version of Eigen downloaded as part of its 3rd party 
deps, so if you need to change it and rebuild you'll need to run:

```bash
cd third-party/tensorflow/tensorflow/lite/tools/make
rm -r downloads/eigen/
./download_dependencies.sh
```
