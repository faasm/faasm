#!/bin/bash

set -e

KEY=$1
VALUE=$2

echo "Updating ${KEY} = ${VALUE}"

kubectl --namespace=faasm patch configmap faasm-config -p '{"data":{"'${KEY}'":"'${VALUE}'"}}"'
