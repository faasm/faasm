#!/bin/bash

set -e

function remove_pyc() {
    find $1 | grep "__pycache__" | xargs rm -rf
    find $1 | grep "\.pyc$" | xargs rm -rf
}

# Pyodide
PYODIDE_ROOT=/usr/local/code/faasm/pyodide
INSTALL_DIR=${PYODIDE_ROOT}/cpython/installs/python-3.7.0
RUNTIME_ROOT=/usr/local/faasm/runtime_root
PY_RUNTIME_ROOT=${RUNTIME_ROOT}/lib/python3.7
SITE_PACKAGES=${PY_RUNTIME_ROOT}/site-packages
NUMPY_DIR=${PYODIDE_ROOT}/packages/numpy/build/numpy-1.15.1/install/lib/python3.7/site-packages/numpy

sudo chown -R ${USER}:${USER} ${INSTALL_DIR}

rm -rf ${RUNTIME_ROOT}/*

sudo mkdir -p ${RUNTIME_ROOT}
sudo chown -R ${USER}:${USER} ${RUNTIME_ROOT}

# Clear out pyc and pycache files for cpython and numpy
remove_pyc ${INSTALL_DIR}
remove_pyc ${NUMPY_DIR}

# Copy everything but remove some stuff we don't want
cp -r ${INSTALL_DIR}/* ${RUNTIME_ROOT}
#rm ${PY_RUNTIME_ROOT}/shutil.py

# Remove the actual lib file
rm ${RUNTIME_ROOT}/lib/libpython*

# Put numpy in place and remove unnecessary bits
cp -r ${NUMPY_DIR} ${SITE_PACKAGES}/
#rm -r ${SITE_PACKAGES}/numpy/tests
#rm -r ${SITE_PACKAGES}/numpy/testing

# Put dummy functions in place
cp -r python/funcs ${RUNTIME_ROOT}
