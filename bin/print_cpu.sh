#!/bin/bash

set -e

CPU_MODEL=$(cat /proc/cpuinfo | grep 'model name' | uniq)

# Strip special characters
CPU_MODEL=$(echo "$CPU_MODEL" | sed 's/model name.*: //g')
CPU_MODEL=$(echo "$CPU_MODEL" | sed 's/ /_/g')
CPU_MODEL=$(echo "$CPU_MODEL" | sed 's/(/_/g')
CPU_MODEL=$(echo "$CPU_MODEL" | sed 's/)/_/g')
CPU_MODEL=$(echo "$CPU_MODEL" | sed 's/@/_/g')

# Print the result
echo $CPU_MODEL
