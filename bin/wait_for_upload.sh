#!/bin/bash

set -e

if [ "$#" -ne 2 ]; then
    echo "Error, must provide upload host and port"
    exit 1
fi

UPLOAD_HOST=$1
UPLOAD_PORT=$2

WAIT_FOR=/tmp/wait-for

if [ ! -f "$WAIT_FOR" ]; then
    pushd /tmp >> /dev/null

    SCRIPT_URL=https://raw.githubusercontent.com/eficode/wait-for/v2.1.3/wait-for
    wget ${SCRIPT_URL}
    chmod +x ${WAIT_FOR}

    popd >> /dev/null
fi

${WAIT_FOR} \
    -t 300 \
    http://${UPLOAD_HOST}:${UPLOAD_PORT}/ping \
    -- echo "Upload server up"

