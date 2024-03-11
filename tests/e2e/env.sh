THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]:-${(%):-%x}}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="${THIS_DIR}/../.."

# Versions
export FAASM_VERSION=$(cat ${PROJ_ROOT}/VERSION)
export FAASMCTL_VERSION=$(cat ${PROJ_ROOT}/requirements.txt | awk -F'faasmctl==' '{ print $2 }' | awk 'NF')

# Faasmctl vars
export FAASM_INI_FILE=${E2E_TESTS_ROOT}/faasm.ini

# Python vars
export VENV_DIR=${E2E_TESTS_ROOT}/venv-e2e-tests

# Point the conan cache source mount to the right directory
export CONAN_CACHE_MOUNT_SOURCE=${PROJ_ROOT}/.conan
