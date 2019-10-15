#!/bin/bash

FUNC_HEADER=faasm-worker.faasm.example.com

set -e

# Get invoke details
ISTIO_IP=$(kubectl get svc istio-ingressgateway --namespace istio-system --output 'jsonpath={.spec.clusterIP}')
#ISTIO_PORT=$(kubectl get svc istio-ingressgateway --namespace istio-system   --output 'jsonpath={.spec.ports[?(@.port==80)].nodePort}')
ISTIO_PORT=80

# Get upload details
UPLOAD_IP=$(kubectl get --namespace=faasm service upload  --output 'jsonpath={.spec.clusterIP}')
UPLOAD_PORT=8002

echo ""
echo "-----        Invoke        -----"
echo ""
echo "curl -H 'Host: ${FUNC_HEADER}' http://${ISTIO_IP}"

echo ""
echo "-----        Upload        -----"
echo ""
echo "curl -X PUT http://${UPLOAD_IP}:${UPLOAD_PORT}/f/<user>/<func> -T <wasm_file>"

echo ""
echo "-----       INI file       -----"
echo ""
echo "[Kubernetes]"
echo "invoke_host = ${ISTIO_IP}"
echo "invoke_port = ${ISTIO_PORT}"
echo "upload_host = ${UPLOAD_IP}"
echo "upload_port = ${UPLOAD_PORT}"
echo ""

