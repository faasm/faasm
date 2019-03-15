#!/bin/bash

rm -rf /usr/local/faasm/runtime_root/*

sudo mkdir -p /usr/local/faasm/runtime_root
sudo chown -R ${USER}:${USER} /usr/local/faasm/runtime_root

# Clear out pyc and pycache
INSTALL_DIR=${HOME}/faasm/pyodide/cpython/installs/python-3.7.0/lib
find ${INSTALL_DIR} | grep "__pycache__" | xargs rm -rf
find ${INSTALL_DIR} | grep "\.pyc$" | xargs rm -rf

cp -r ${INSTALL_DIR} /usr/local/faasm/runtime_root