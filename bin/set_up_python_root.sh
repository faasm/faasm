#!/bin/bash

set -e

function remove_pyc() {
    find $1 | grep "__pycache__" | xargs rm -rf
    find $1 | grep "\.pyc$" | xargs rm -rf
}

# ----------------------
# CPYTHON
# ----------------------

PROJ_ROOT=/usr/local/code/faasm
PYODIDE_ROOT=${PROJ_ROOT}/pyodide
INSTALL_DIR=${PYODIDE_ROOT}/cpython/installs/python-3.7.0
RUNTIME_ROOT=/usr/local/faasm/runtime_root
PY_RUNTIME_ROOT=${RUNTIME_ROOT}/lib/python3.7

if [ ! -d "$RUNTIME_ROOT" ]; then
    sudo mkdir -p ${RUNTIME_ROOT}
    sudo chown -R ${USER}:${USER} ${RUNTIME_ROOT}
fi

rm ${RUNTIME_ROOT}/funcs/*.py
rm -r ${RUNTIME_ROOT}/include/*
rm -r ${RUNTIME_ROOT}/lib/*

# Clear out pyc and pycache files for cpython and numpy
remove_pyc ${INSTALL_DIR}

# Copy everything
cp -r ${INSTALL_DIR}/* ${RUNTIME_ROOT}

# Remove the actual lib file
rm ${RUNTIME_ROOT}/lib/libpython*

# ----------------------
# PACKAGES
# ----------------------

SITE_PACKAGES=${PY_RUNTIME_ROOT}/site-packages
NUMPY_DIR=${PYODIDE_ROOT}/packages/numpy/build/numpy-1.15.1/install/lib/python3.7/site-packages/numpy
PERF_DIR=${PYODIDE_ROOT}/packages/perf/build/perf-1.6.0/install/lib/python3.7/site-packages/perf
PERFORMANCE_DIR=${PYODIDE_ROOT}/packages/performance/build/performance-0.7.0/install/lib/python3.7/site-packages/performance

# Packages in place
remove_pyc ${NUMPY_DIR}
remove_pyc ${PERF_DIR}
remove_pyc ${PERFORMANCE_DIR}
cp -r ${NUMPY_DIR} ${SITE_PACKAGES}/
cp -r ${PERF_DIR} ${SITE_PACKAGES}/
cp -r ${PERFORMANCE_DIR} ${SITE_PACKAGES}/

# Put dummy functions in place
cp -r ${PROJ_ROOT}/python/funcs ${RUNTIME_ROOT}
