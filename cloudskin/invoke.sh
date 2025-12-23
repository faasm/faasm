#!/bin/bash

set -euo pipefail

# First invoke a task that just occupies space and sleeps
faasmctl invoke omp elastic_imagesim --cmdline "dataset/dataset 8 0" &
sleep 3
faasmctl invoke omp elastic_imagesim --cmdline "dataset/dataset 4 0" &

sleep 0.5

# Then invoke an elastic image scale
faasmctl invoke omp elastic_imagesim --cmdline "dataset/dataset 4 2" --elastic
