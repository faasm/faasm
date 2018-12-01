#!/bin/bash

set -e

kubectl --namespace=faasm patch configmap faasm-config -p'{"data": {"$1":"$2"} }'
