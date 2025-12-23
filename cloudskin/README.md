# CloudSkin Image Similarity Pipeline

This document includes notes on how to run the image similarity pipeline for
CloudSkin on top of Granny.

Before starting, make sure you have a working Granny cluster:

```bash
# How many vCPUs per-worker.
export FAASM_OVERRIDE_CPU_COUNT=8
faasmctl deploy.compose --workers=1 --mount-source .
```

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
cargo build --target=wasm32-wasip1 --release

# We need to disable some automatic ref-types introduced in LLVM 19+.
wasm-opt -O \
    -o ./target/wasm32-wasip1/debug/similarity_test.wasm
    ./target/wasm32-wasip1/debug/similarity_test_opt.wasm
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
faasmctl upload.wasm cloudskin imagesim --wasm-file ~/git/ZikBurns/ExampleImageSimilarity/code/target/wasm32-wasip1/debug/similarity_test_opt.wasm

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

## Interacting with training data in MinIO

In the real pipeline we will use training data available in a MinIO cluster. To
interact with a running cluster, you may use the `mc` tool:

```bash
docker run -it --entrypoint=/bin/sh --network host minio/mc
mc alias set cloudskin http://localhost:7000 minio99 minio123
mc alias set faasm http://localhost:9000 minio minio123
# FIXME: Faasm cannot currently access the MinIO server that is listening on
# localhost, as Faasm is deployed inside its own compose cluster.
mc mb faasm/dataset
mc cp -r cloudskin/pepe-bucket/output/dataset
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
