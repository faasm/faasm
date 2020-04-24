#!/bin/bash

#CALLING_DIR=$(pwd)
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
pushd ${THIS_DIR} >> /dev/null

# ----------------------------
# Virtualenv
# ----------------------------

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

export VIRTUAL_ENV_DISABLE_PROMPT=1
source venv/bin/activate

# Allow overriding the invoke tasks file
# INVOKE_ROOT=${INVOKE_ROOT:-$THIS_DIR}  
# alias inv="inv -r ${INVOKE_TASKS}"

# Ensure both the Faasm Python code is accessible, as well as the calling dir
export PYTHONPATH="${PYTHONPATH}:${CALLING_DIR}:${THIS_DIR}/tasks"

# ----------------------------
# Invoke tab-completion (http://docs.pyinvoke.org/en/stable/invoke.html#shell-tab-completion)
# ----------------------------

_complete_invoke() {
    local candidates
    candidates=`invoke --complete -- ${COMP_WORDS[*]}`
    COMPREPLY=( $(compgen -W "${candidates}" -- $2) )
}

# complete -F _complete_invoke -o default invoke inv

# ----------------------------
# Environment vars
# ----------------------------

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

export LOG_LEVEL=debug
export FAASM_ROOT=$(pwd)
export FAASM_VERSION=$(cat VERSION)
export FAASM_CMAKE_TOOLCHAIN=$FAASM_ROOT/toolchain/FaasmToolchain.cmake
export FAASM_BASH_TOOLCHAIN=$FAASM_ROOT/toolchain/env.sh

export PS1="(faasm) $PS1"

alias fm="inv"

# Container builds
if [[ -d /faasm/build ]]; then
  export PATH=/faasm/build/bin:$PATH
fi

# Bare metal
if [[ -d "${THIS_DIR}/build" ]]; then
  export PATH=${THIS_DIR}/build/bin:$PATH
fi

# Native MPI
export PATH=/usr/local/faasm/openmpi/bin:$PATH

echo ""
echo "----------------------------------"
echo "Faasm CLI"
echo "Version: ${FAASM_VERSION}"
echo "Project root: ${THIS_DIR}"
echo "----------------------------------"
echo ""

popd >> /dev/null
