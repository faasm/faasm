#!/bin/bash

set -euo pipefail

# First invoke a task that just occupies space and sleeps
faasmctl invoke cloudskin elastic_imagesim --cmdline "8 0" &
sleep 1
faasmctl invoke cloudskin elastic_imagesim --cmdline "4 0" &

sleep 0.5

# Then invoke an elastic image scale
faasmctl invoke cloudskin elastic_imagesim --cmdline "4 2" --elastic
