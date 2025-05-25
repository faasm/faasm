# CloudSkin Image Similarity Pipeline

This document includes notes on how to run the image similarity pipeline for
CloudSkin on top of Granny.

Before starting, make sure you have a working Granny cluster.

## Build Image Similarity Code

We first need to build the image similarity code:

```bash
git clone git@github.com:ZikBurns/ExampleImageSimilarity.git
cd ExampleImageSimilarity/code

# Prepare data.
source venv/bin/activate
python prepare_dataset.py
python train.py

# Build similarity kernel.
cargo build --target=wasm32-wasip1
```

then we need the Granny shim:

```bash
source ./bin/workon.sh
faasmctl cli.cpp
inv func omp elastic_imagesim
exit
```

Now we can upload the WASM modules:

```bash
# Image similarity.
faasmctl upload.wasm cloudskin imagesim --wasm-file ~/git/ZikBurns/ExampleImageSimilarity/code/target/wasm32-wasip1/debug/similarity_test.wasm

# Granny shim.
faasmctl upload.wasm cloudskin elastic_imagesim --wasm-file ./dev/faasm-local/wasm/omp/elastic_imagesim/function.wasm
```

## Upload training data to cluster

Right now, we manually copy the training data from the image similarity repo
into Granny's runtime sysroot.

```bash
cp -r ~/git/ZikBurns/ExampleImageSimilarity/code/knn/ dev/faasm-local/runtime_root/
cp -r ~/git/ZikBurns/ExampleImageSimilarity/code/data/ dev/faasm-local/runtime_root/
```

## Run the experiments

Before running the experiments, you may want to open a new terminal to monitor
the execution:

```bash
source ./bin/workon.sh
faasmctl monitor.planner
```

then you may run the execution with:

```bash
./invoke.sh
```
