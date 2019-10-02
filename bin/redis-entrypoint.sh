#!/bin/bash

set -e

# Start the redis server in background
redis-server &
SERVER_PID=$!

sleep 1s

# Clear the store
redis-cli flushall

# Wait for cluster to finish
wait $SERVER_PID
