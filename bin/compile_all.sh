#!/bin/bash

set -e

./bin/compile.sh simon chain
./bin/compile.sh simon dummy
./bin/compile.sh simon echo
./bin/compile.sh simon x2
