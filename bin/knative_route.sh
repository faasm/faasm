#!/bin/bash


set -e

KNATIVE_HOST=$(kn service describe faasm-worker -o url -n faasm | cut -c 8-)

function service_ip {
    local svc_ns=$1
    local svc_name=$2

    # Try getting public IP
    local ip_res=$(kubectl get -n ${svc_ns} service ${svc_name} -o 'jsonpath={.status.loadBalancer.ingress[0].ip}')

    # Get private IP if failed
    if [[ -z "$ip_res" ]]; then
        ip_res=$(kubectl get -n ${svc_ns} service ${svc_name} -o 'jsonpath={.spec.clusterIP}')
    fi

    echo $ip_res
}

# Get invoke details
ISTIO_IP=$(service_ip istio-system istio-ingressgateway)
ISTIO_PORT=80

UPLOAD_IP=$(service_ip faasm upload)
UPLOAD_PORT=8002

echo ""
echo "-----        Invoke        -----"
echo ""
echo "curl -H 'Host: ${KNATIVE_HOST}' http://${ISTIO_IP}"

echo ""
echo "-----        Upload        -----"
echo ""
echo "curl -X PUT http://${UPLOAD_IP}:${UPLOAD_PORT}/f/<user>/<func> -T <wasm_file>"

echo ""
echo "-----       INI file       -----"
echo ""
echo "[Faasm]"
echo "invoke_host = ${ISTIO_IP}"
echo "invoke_port = ${ISTIO_PORT}"
echo "upload_host = ${UPLOAD_IP}"
echo "upload_port = ${UPLOAD_PORT}"
echo "knative_host = ${KNATIVE_HOST}"
echo ""

