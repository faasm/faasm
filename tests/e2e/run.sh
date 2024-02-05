#!/bin/bash

# Configure env. vars
export E2E_TESTS_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]:-${(%):-%x}}" )" >/dev/null 2>&1 && pwd )"
pushd ${E2E_TESTS_ROOT} >> /dev/null
source ./env.sh

# Install faasmctl
pip3 install faasmctl==${FAASMCTL_VERSION}

# Start Faasm cluster and silence the output to make it easier to read the logs
if [ "${FAASM_MOUNT_SOURCE}" == "on" ]; then
    faasmctl deploy.compose --mount-source ${FAASM_SOURCE}
    faasmctl cli.faasm --cmd "./bin/inv_wrapper.sh dev.tools --build Release"
    faasmctl restart -s upload -s worker
else
    faasmctl deploy.compose --clean
fi

echo "======================================================================="
echo "                         FAASM E2E TESTS                               "
echo "======================================================================="

# Run tests
GLOBIGNORE="env.sh:run.sh"
E2E_TESTS_DIR=${E2E_TESTS_ROOT}/tests
export SKIPPED_TEST_RET_VAL=222

rc=0
failed_tests=""
failed_test_num=0
skipped_tests=""
skipped_test_num=0
total_test_num=0

pushd ${E2E_TESTS_DIR} >> /dev/null

for test_case in *.sh; do
    echo "-----------------------------------------------------------------------"
    echo "        Running test case: ${test_case}"
    echo "-----------------------------------------------------------------------"
    # Discard the logs, but capture the return value
    echo "Executing: ${E2E_TESTS_DIR}/${test_case}"
    this_test=$(${E2E_TESTS_DIR}/${test_case})
    this_rc=$?
    echo "RC: ${this_rc}"
    echo "Output: ${this_test}"
    case $this_rc in
        0)
            echo "Success!";;
        ${SKIPPED_TEST_RET_VAL})
            echo "Skipped!"
            skipped_tests="${skipped_tests}\n${test_case}"
            skipped_test_num=$((${skipped_test_num}+1));;
        *)
            echo "Failed!"
            failed_tests="${failed_tests}\n${test_case}"
            rc=1
            failed_test_num=$((${failed_test_num}+1));;
    esac
    total_test_num=$((total_test_num+1))
done

popd >> /dev/null

# Print results
success_test_num=$((total_test_num-failed_test_num-skipped_test_num))
echo "======================================================================="
if [ "$rc" -eq "0" ]; then
    echo "                  FAASM E2E TESTS: SUCCESS!                            "
else
    echo "                  FAASM E2E TESTS: Failed!                             "
fi
echo "-----------------------------------------------------------------------"
echo "Succesful tests: ${success_test_num}/${total_test_num}"
echo -e "Skipped tests: ${skipped_test_num}/${total_test_num}${skipped_tests}"
echo -e "Failed tests: ${failed_test_num}/${total_test_num}${failed_tests}"
echo "======================================================================="


# Delete Faasm cluster
faasmctl delete

popd >> /dev/null

exit ${rc}
