#!/bin/bash

set -e

pushd /faasm/build >> /dev/null

# Run codegen for some functions
SGD_OBJ_FILE=/usr/local/faasm/object/sgd/reuters_svm/function.wasm.o
if [[ -f "${SGD_OBJ_FILE}" ]]; then
  echo "Special object files already exist"
else
  ORIGINAL_FUNCTION_STORAGE=$FUNCTION_STORAGE
  export FUNCTION_STORAGE=local
  echo "Temporarily changing function storage from ${ORIGINAL_FUNCTION_STORAGE} to local"

  echo "Generating object files"
  ./bin/codegen_func sgd reuters_svm
  ./bin/codegen_func python py_func

  echo "Reverting function storage from local to ${ORIGINAL_FUNCTION_STORAGE}"
  export FUNCTION_STORAGE=${ORIGINAL_FUNCTION_STORAGE}
fi

popd >> /dev/null

pushd /usr/local/code/faasm >> /dev/null

echo "Setting up cgroup"
./bin/cgroup.sh

echo "Setting up namespaces"
./bin/netns.sh ${THREADS_PER_WORKER}

popd >> /dev/null

# Continue with normal command
exec "$@"