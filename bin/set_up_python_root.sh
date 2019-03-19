#!/bin/bash

# Pyodide
PYODIDE_ROOT=/usr/local/code/pyodide
INSTALL_DIR=${PYODIDE_ROOT}/cpython/installs/python-3.7.0
sudo chown -R ${USER}:${USER} ${INSTALL_DIR}
SITE_PACKAGES=/usr/local/faasm/runtime_root/lib/python3.7/site-packages/

# Emscripten
#INSTALL_DIR=/usr/local/code/faasm/cpython-emscripten/installs/python-3.5.2/

rm -rf /usr/local/faasm/runtime_root/*

sudo mkdir -p /usr/local/faasm/runtime_root
sudo chown -R ${USER}:${USER} /usr/local/faasm/runtime_root

# Clear out pyc and pycache
find ${INSTALL_DIR} | grep "__pycache__" | xargs rm -rf
find ${INSTALL_DIR} | grep "\.pyc$" | xargs rm -rf

cp -r ${INSTALL_DIR}/* /usr/local/faasm/runtime_root

# Remove the actual lib file
rm /usr/local/faasm/runtime_root/lib/libpython*

# Put numpy in place
NUMPY_DIR=${PYODIDE_ROOT}/packages/numpy/build/numpy-1.15.1/install/lib/python3.7/site-packages/numpy
cp -r ${NUMPY_DIR} ${SITE_PACKAGES}

