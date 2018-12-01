#!/bin/bash

set -e

DATA_STR="{\"data\": { \"$1\" : \"$2\" } }"

kubectl --namespace=faasm patch configmap faasm-config -p"'"$DATA_STR"'"
