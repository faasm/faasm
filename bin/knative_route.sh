#!/bin/bash

FUNC_NAME=faasm-test

set -e

# Print details of the top-level gateway
echo ""
echo "----- Istio Gateway -----"
kubectl get svc istio-ingressgateway --namespace istio-system

# Print details of the function route
echo ""
echo "----- Function Route -----"
kubectl --namespace faasm get route ${FUNC_NAME}

# Get IP and host for this service
IP_ADDRESS=$(kubectl get node  --output 'jsonpath={.items[0].status.addresses[0].address}')

PORT=$(kubectl get svc istio-ingressgateway --namespace istio-system   --output 'jsonpath={.spec.ports[?(@.port==80)].nodePort}')

HOST=${IP_ADDRESS}:${PORT}
FUNC_HOST=${FUNC_NAME}.faasm.example.com
echo ""
echo "----- Request details -----"
echo "HOST = ${HOST}"
echo "FUNCTION HOST = ${FUNC_HOST}"

echo ""
echo "----- Curl -----"
echo "CMD: "
echo "curl -H 'Host: ${FUNC_HOST}' http://${HOST}"

echo ""
echo "----- Upload -----"
kubectl get service upload --namespace=faasm