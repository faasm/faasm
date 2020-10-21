#!/bin/bash

# NOTE - this is primary designed to be run inside the cli Docker container

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="${THIS_DIR}/.."
pushd ${PROJ_ROOT} >> /dev/null

# ----------------------------
# Container-specific settings
# ----------------------------

if [[ -z "$FAASM_DOCKER" ]]; then
    # Use containerised redis
    alias redis-cli="redis-cli -h redis"

    # Build binaries on path
    export PATH=/build/faasm/bin:$PATH
fi

# ----------------------------
# Virtualenv
# ----------------------------

if [ ! -d "venv" ]; then
    python3 -m venv venv 
fi

export VIRTUAL_ENV_DISABLE_PROMPT=1
source venv/bin/activate

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

VERSION_FILE=${PROJ_ROOT}/VERSION
export LOG_LEVEL=debug
export FAASM_ROOT=$(pwd)
export FAASM_VERSION=$(cat ${VERSION_FILE})

export PS1='(faasm) ${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '

# -----------------------------
# Splash
# -----------------------------

echo ""
echo "----------------------------------"
echo "Faasm CLI"
echo "Version: ${FAASM_VERSION}"
echo "Project root: ${PROJ_ROOT}"
echo "----------------------------------"
echo ""

popd >> /dev/null
