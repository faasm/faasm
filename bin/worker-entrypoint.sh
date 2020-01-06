#!/bin/bash

set -e

pushd /faasm/build >> /dev/null

ORIGINAL_FUNCTION_STORAGE=$FUNCTION_STORAGE
export FUNCTION_STORAGE=local
echo "Temporarily changing function storage from ${ORIGINAL_FUNCTION_STORAGE} to local"

# Run codegen for some functions if needed
if [[ "${SGD_CODEGEN}" == "on" ]]; then
  [ -f "/usr/local/faasm/object/sgd/reuters_svm/function.wasm.o" ] || ./bin/codegen_func sgd reuters_svm
fi

if [[ "${TF_CODEGEN}" == "on" ]]; then
  [ -f "/usr/local/faasm/object/tf/image/function.wasm.o" ] || ./bin/codegen_func tf image
fi

if [[ "${PYTHON_CODEGEN}" == "on" ]]; then
  echo "Generating python object files"
  ./bin/codegen_func python py_func
  ./bin/codegen_shared_obj /usr/local/faasm/runtime_root/lib/python3.7
fi

echo "Reverting function storage from local to ${ORIGINAL_FUNCTION_STORAGE}"
export FUNCTION_STORAGE=${ORIGINAL_FUNCTION_STORAGE}

popd >> /dev/null

pushd /usr/local/code/faasm >> /dev/null

echo "Setting up cgroup"
./bin/cgroup.sh

echo "Setting up namespaces"
./bin/netns.sh ${THREADS_PER_WORKER}

popd >> /dev/null

# Continue with normal command
exec "$@"