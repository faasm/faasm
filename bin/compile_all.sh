#!/bin/bash

set -e

python3 bin/compile.py simon chain
python3 bin/compile.py simon cpu
python3 bin/compile.py simon dummy
python3 bin/compile.py simon echo
python3 bin/compile.py simon x2
python3 bin/compile.py simon curl --libcurl
