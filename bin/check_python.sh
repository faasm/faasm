#!/bin/bash

set -e

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} >> /dev/null

# Check all files
FILES_TO_CHECK=$(git ls-files -- "*.py")

# Run black
black --check ${FILES_TO_CHECK}

# Run flake8
python3 -m flake8 ${FILES_TO_CHECK}

popd >> /dev/null

