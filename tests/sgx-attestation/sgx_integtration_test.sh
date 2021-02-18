#!/bin/bash
set -e # fail on error

USER="demo"
UPLOAD_FUNCS="hello chain_named_a chain_named_b chain_named_c"
INVOKE_FUNCS="hello chain_named_a"

echo "################################################################################"
echo "# Test: Policy Generation ######################################################"
echo "################################################################################"

for f in $UPLOAD_FUNCS; do
    echo "- Function $f --------------------------------------------------------"
    inv -r faasmcli/faasmcli policy $USER $f
    echo -e "\n"
done

echo -e "\n"

echo "################################################################################"
echo "# Test: Function and Policy Upload #############################################"
echo "################################################################################"

for f in $UPLOAD_FUNCS; do
    echo "- Function $f --------------------------------------------------------"
    inv -r faasmcli/faasmcli upload $USER $f --sgx
    echo -e "\n"
done

echo -e "\n"

echo "################################################################################"
echo "# Test: Function Invocation ####################################################"
echo "################################################################################"

for f in $INVOKE_FUNCS; do
    echo "- Function $f --------------------------------------------------------"
    inv -r faasmcli/faasmcli invoke $USER $f --sgx
done

# if we reach this withour error: nice!
echo "################################################################################"
echo "# ALL TESTS PASSED #############################################################"
echo "################################################################################"
