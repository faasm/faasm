#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide deploy mode (local/ remote)"
    exit 1
fi

DEPLOY_MODE=$1

THIS_DIR=$(dirname $(readlink -f $0))
K8S_CONF_DIR=${THIS_DIR}/../k8s

COMMON_CONF=${K8S_CONF_DIR}/common
KNATIVE_CONF=${K8S_CONF_DIR}/knative
LOCAL_CONF=${K8S_CONF_DIR}/local
REMOTE_CONF=${K8S_CONF_DIR}/remote

kubectl apply -f ${COMMON_CONF}/namespace.yml
kubectl apply -f ${COMMON_CONF}
kubectl apply -f ${KNATIVE_CONF}

if [ "$DEPLOY_MODE" == "remote" ];
then
    echo "Running remote K8s scripts"
    kubectl apply -f ${REMOTE_CONF}
else
    echo "Running local K8s scripts"
    kubectl apply -f ${LOCAL_CONF}
fi
