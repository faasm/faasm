#!/bin/bash

set -e

echo "---------------------------------"
echo "Starting provisioning minimal GCP..."
echo "---------------------------------"
echo ""

# Create a few instances
function create_vm() {
    echo "Provisioning ${1}"
    gcloud compute instances create ${1}    \
        --image-project ubuntu-os-cloud     \
        --image-family ubuntu-1804-lts      \
        --boot-disk-size 20GB             \
        --machine-type e2-standard-4
}

# Worker machines
create_vm faasm-worker-1
create_vm faasm-worker-2
create_vm faasm-worker-3

# Upload machine 
create_vm faasm-upload
    
# List instances
echo "---------------------------------"
echo "Provisioning complete"
echo "---------------------------------"
echo ""
gcloud compute instances list

