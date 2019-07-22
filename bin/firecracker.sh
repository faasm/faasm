#!/bin/bash

set -e

FIRECRACKER_DIR=${HOME}/faasm/firecracker
API_SOCKET=/tmp/firecracker.socket
KERNEL=hello-vmlinux.bin
ROOTFS=hello-rootfs.ext4

pushd ${FIRECRACKER_DIR} >> /dev/null

# Need to make sure API socket removed
rm -f ${API_SOCKET}

# Start firecracker (parentheses required)
( ./firecracker --api-sock ${API_SOCKET} & )
FIRECRACKER_PID=$!
echo "Firecracker running with PID ${FIRECRACKER_PID}"

# Set the kernel
curl --unix-socket ${API_SOCKET} -i \
        -X PUT 'http://localhost/boot-source'   \
        -H 'Accept: application/json'           \
        -H 'Content-Type: application/json'     \
        -d "{
              \"kernel_image_path\": \"${KERNEL}\",
              \"boot_args\": \"console=ttyS0 reboot=k panic=1 pci=off\"
         }"

# Set the rootfs
curl --unix-socket ${API_SOCKET} -i \
    -X PUT 'http://localhost/drives/rootfs' \
    -H 'Accept: application/json'           \
    -H 'Content-Type: application/json'     \
    -d "{
          \"drive_id\": \"rootfs\",
          \"path_on_host\": \"${ROOTFS}\",
          \"is_root_device\": true,
          \"is_read_only\": false
     }"

# Start the VM
curl --unix-socket ${API_SOCKET} -i \
    -X PUT 'http://localhost/actions'       \
    -H  'Accept: application/json'          \
    -H  'Content-Type: application/json'    \
    -d '{
        "action_type": "InstanceStart"
     }'

# Bring Firecracker to foreground
fg

# Kill Firecracker
kill -9 ${FIRECRACKER_PID}
pkill -f firecracker

popd >> /dev/null