#!/bin/bash

set -e

# More info in tutorial: https://knative.dev/docs/install/getting-started-knative-app/
kubectl apply --filename knative/service.yaml
