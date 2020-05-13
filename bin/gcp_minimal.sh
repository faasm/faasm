#!/bin/bash

set -e

echo "---------------------------------"
echo "Starting provisioning minimal GCP..."
echo "---------------------------------"
echo ""

# Create a few instances
function create_vm() {
    echo "Provisioning ${1} VM"
    gcloud compute instances create ${1}    \
        --image-project ubuntu-os-cloud     \
        --image-family ubuntu-1804-lts      \
        --machine-type e2-standard-4

#    echo "Attaching ${1}-disk to ${1} VM"
#    gcloud compute instances attach-disk ${1} \
#        --disk ${1}-disk
#
#    echo "Provisioning ${1} disk"
#    gcloud compute disks create ${1}-disk \
#        --size 20GB \
#        --type pd-ssd
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

