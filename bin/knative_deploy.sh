#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
K8S_CONF_DIR=${THIS_DIR}/../k8s
COMMON_CONF=${K8S_CONF_DIR}/common
KNATIVE_CONF=${K8S_CONF_DIR}/knative

kubectl apply -f ${COMMON_CONF}/namespace.yml
kubectl apply -f ${COMMON_CONF}
kubectl apply -f ${KNATIVE_CONF}
