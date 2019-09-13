#!/bin/bash

set -e

mkdir -p downloads
pushd downloads

echo "Downloading model and labels"
wget https://storage.googleapis.com/download.tensorflow.org/models/mobilenet_v1_2018_02_22/mobilenet_v1_1.0_224.tgz
wget https://storage.googleapis.com/download.tensorflow.org/models/mobilenet_v1_1.0_224_frozen.tgz

echo "Extracting"
tar -xf mobilenet_v1_1.0_224.tgz
tar -xf mobilenet_v1_1.0_224_frozen.tgz

mv mobilenet_v1_1.0_224/labels.txt ..
mv mobilenet_v1_1.0_224.tflite ..

popd

rm -rf downloads
