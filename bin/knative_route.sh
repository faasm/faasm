#!/bin/bash

FUNC_HEADER=faasm-worker.faasm.example.com

set -e

# Get invoke details
MASTER_IP=$(kubectl get node --output 'jsonpath={.items[0].status.addresses[0].address}')
MASTER_PORT=$(kubectl get svc istio-ingressgateway --namespace istio-system   --output 'jsonpath={.spec.ports[?(@.port==80)].nodePort}')
MASTER_HOST=${MASTER_IP}:${MASTER_PORT}

# Get upload details
UPLOAD_IP=$(kubectl get --namespace=faasm service upload  --output 'jsonpath={.spec.clusterIP}')
UPLOAD_EXTERNAL_IP=$(kubectl get --namespace=faasm service upload  --output 'jsonpath={.spec.externalIPs[0]}')
UPLOAD_PORT=8002

echo ""
echo "-----        Invoke        -----"
echo ""
echo "curl -H 'Host: ${FUNC_HEADER}' http://${MASTER_HOST}"

echo ""
echo "-----        Upload        -----"
echo ""
echo "curl -X PUT http://${UPLOAD_IP}:${UPLOAD_PORT}/f/<user>/<func> -T <wasm_file>"

echo ""
echo "-----       INI file       -----"
echo ""
echo "[Kubernetes]"
echo "invoke_host = ${MASTER_IP}"
echo "invoke_port = ${MASTER_PORT}"
echo "upload_host = ${UPLOAD_IP}"
echo "upload_port = ${UPLOAD_PORT}"
echo ""
