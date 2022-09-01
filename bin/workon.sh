#!/bin/bash

# NOTE - this is primary designed to be run inside the cli Docker container

# ----------------------------
# Container-specific settings
# ----------------------------

MODE="undetected"
if [[ -z "$FAASM_DOCKER" ]]; then

    THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    if [ "$(ps -o comm= -p $$)" = "zsh" ]; then
        THIS_DIR="$( cd "$( dirname "${ZSH_ARGZERO}" )" >/dev/null 2>&1 && pwd )"
    fi
    echo $THIS_DIR
    PROJ_ROOT="${THIS_DIR}/.."
    #PROJ_ROOT="${THIS_DIR}/"
    VENV_PATH="${PROJ_ROOT}/venv-bm"

    # Normal terminal
    MODE="terminal"
else
    # Running inside the container, we know the project root
    PROJ_ROOT="/usr/local/code/faasm"
    VENV_PATH="${PROJ_ROOT}/venv"

    # Use containerised redis
    alias redis-cli="redis-cli -h redis"

    MODE="container"
fi

pushd ${PROJ_ROOT} >> /dev/null

# ----------------------------
# Virtualenv
# ----------------------------

if [ ! -d ${VENV_PATH} ]; then
    ${PROJ_ROOT}/bin/create_venv.sh
fi

export VIRTUAL_ENV_DISABLE_PROMPT=1
source ${VENV_PATH}/bin/activate

# Aliases for invoke
alias inv="inv -r faasmcli/faasmcli"
alias invoke="invoke -r faasmcli/faasmcli"

# ----------------------------
# Invoke tab-completion
# (http://docs.pyinvoke.org/en/stable/invoke.html#shell-tab-completion)
# ----------------------------

_complete_invoke() {
    local candidates
    candidates=`invoke --complete -- ${COMP_WORDS[*]}`
    COMPREPLY=( $(compgen -W "${candidates}" -- $2) )
}

# If running from zsh, run autoload for tab completion
if [ "$(ps -o comm= -p $$)" = "zsh" ]; then
    autoload bashcompinit
    bashcompinit
fi
complete -F _complete_invoke -o default invoke inv

# ----------------------------
# Environment vars
# ----------------------------

# Related to building outisde a container
VERSION_FILE=${PROJ_ROOT}/VERSION
export LOG_LEVEL=debug
export FAASM_ROOT=$(pwd)
export FAASM_VERSION=$(cat ${VERSION_FILE})

if [[ "$MODE" == "terminal" ]]; then
    export FAASM_BUILD_DIR=$(pwd)/dev/native/build
    export FAASM_INSTALL_DIR=$(pwd)/dev/native/install
    export FAASM_LOCAL_DIR=$(pwd)/dev/faasm-local
else
    export FAASM_BUILD_DIR=/build/faasm
fi

# Build binaries on path
export PATH=${FAASM_BUILD_DIR}/bin:$PATH

if [ -z "$PS1_old" ]; then
  # PS1_old is not set
  export PS1_old=$PS1
fi
export PS1="(faasm) $PS1_old"

# -----------------------------
# Splash
# -----------------------------

echo ""
echo "----------------------------------"
echo "Faasm CLI"
echo "Version: ${FAASM_VERSION}"
echo "Project root: ${PROJ_ROOT}"
echo "Mode: ${MODE}"
echo "----------------------------------"
echo ""

popd >> /dev/null
