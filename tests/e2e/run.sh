#!/bin/bash

# Configure env. vars
export E2E_TESTS_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]:-${(%):-%x}}" )" >/dev/null 2>&1 && pwd )"
pushd ${E2E_TESTS_ROOT} >> /dev/null
source ./env.sh

# Install faasmctl
pip3 install faasmctl==${FAASMCTL_VERSION}

# Start Faasm cluster
if [ "${FAASM_MOUNT_SOURCE}" == "on" ]; then
    faasmctl deploy.compose --mount-source ${FAASM_SOURCE}
    faasmctl cli.faasm --cmd "./bin/inv_wrapper.sh dev.tools --build Release"
    faasmctl restart -s upload -s worker
else
    faasmctl deploy.compose
fi

echo "======================================================================="
echo "                         FAASM E2E TESTS                               "
echo "======================================================================="

# Run tests
rc=0
GLOBIGNORE="env.sh:run.sh"
E2E_TESTS_DIR=${E2E_TESTS_ROOT}/tests
failed_tests=""
failed_test_num=0
total_test_num=0

pushd ${E2E_TESTS_DIR} >> /dev/null

for test_case in *.sh; do
    echo "-----------------------------------------------------------------------"
    echo "        Running test case: ${test_case}"
    echo "-----------------------------------------------------------------------"
    if ${E2E_TESTS_DIR}/${test_case}; then
        echo "Success!"
    else
        echo "Failed!"
        failed_tests="${failed_tests}\n${test_case}"
        rc=1
        failed_test_num=$((${failed_test_num}+1))
    fi
    total_test_num=$((total_test_num+1))
done

popd >> /dev/null

# Print results
success_test_num=$((total_test_num-failed_test_num))
echo "======================================================================="
if [ "$rc" -eq "0" ]; then
    echo "                  FAASM E2E TESTS: SUCCESS!                            "
else
    echo "                  FAASM E2E TESTS: Failed!                             "
fi
echo "-----------------------------------------------------------------------"
echo "Succesful tests: ${success_test_num}/${total_test_num}"
echo -e "Failed tests: ${failed_tests}"
echo "======================================================================="


# Delete Faasm cluster
faasmctl delete

popd >> /dev/null

exit ${rc}
