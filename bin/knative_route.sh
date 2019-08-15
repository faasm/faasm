#!/bin/bash

FUNC_NAME=faasm-worker
FUNC_HOST=${FUNC_NAME}.faasm.example.com

set -e

echo "--------------------------------"
echo "-----    Top-level info    -----"
echo "--------------------------------"

# Print details of the top-level gateway
echo ""
echo "-----     Istio Gateway    -----"
echo ""
kubectl get svc istio-ingressgateway --namespace istio-system

# Print details of the function route
echo ""
echo "-----    Function Route    -----"
echo ""
kubectl --namespace faasm get route ${FUNC_NAME}

# Get IP and host for the K8s master node
MASTER_IP=$(kubectl get node  --output 'jsonpath={.items[0].status.addresses[0].address}')

MASTER_PORT=$(kubectl get svc istio-ingressgateway --namespace istio-system   --output 'jsonpath={.spec.ports[?(@.port==80)].nodePort}')

MASTER_HOST=${MASTER_IP}:${MASTER_PORT}

echo ""
echo "--------------------------------"
echo "----- Function Invocation  -----"
echo "--------------------------------"
echo ""
echo "curl -H 'Host: ${FUNC_HOST}' http://${MASTER_HOST}"

echo ""
echo "--------------------------------"
echo "-----        Upload        -----"
echo "--------------------------------"
echo ""
UPLOAD_IP=$(kubectl get --namespace=faasm service upload  --output 'jsonpath={.spec.clusterIP}')
UPLOAD_EXTERNAL_IP=$(kubectl get --namespace=faasm service upload  --output 'jsonpath={.spec.externalIPs[0]}')
UPLOAD_PORT=8002

echo "curl -X PUT http://${UPLOAD_IP}:${UPLOAD_PORT}/f/<user>/<func> -T <wasm_file>"

echo ""
echo "--------------------------------"
echo "-----    External host     -----"
echo "--------------------------------"
echo ""
echo "${UPLOAD_EXTERNAL_IP}"
echo ""
