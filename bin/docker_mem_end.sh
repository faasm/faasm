#!/bin/bash

set -e

# Note, these will fail if there are no containers to remove
echo "Finishing Docker mem"
docker ps -aq | xargs docker stop || true
docker ps -aq | xargs docker rm || true
