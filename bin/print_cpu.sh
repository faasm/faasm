#!/bin/bash

set -e

echo "$(cat /proc/cpuinfo | grep 'model name' | uniq | sed 's/model name.*: //g' | sed 's/ /_/g')"
