#!/bin/bash

set -e

POD_IPS=$(kubectl -n faasm get pods -l app=redis-cluster -o jsonpath='{range.items[*]}{.status.podIP}:6379 ')

echo "Setting up cluster with IPs ${POD_IPS}"

kubectl -n faasm exec -it redis-cluster-0 -- redis-cli --cluster create --cluster-replicas 0 ${POD_IPS}
