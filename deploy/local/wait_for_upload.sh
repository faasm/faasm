#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Error, must provide upload host and port"
    exit 1
fi

UPLOAD_HOST=$1
UPLOAD_PORT=$2
URL=http://${UPLOAD_HOST}:${UPLOAD_PORT}/ping

HTTP_STATUS="0"
CHECK_COUNT=0
SLEEP_TIME=5
MAX_CHECKS=120

while [ true ]
do
    echo "Checking upload server at ${URL}"
    HTTP_STATUS=$(curl -s -o /dev/null -w "%{http_code}" "${URL}")

    ((CHECK_COUNT++))

    if [[ "$HTTP_STATUS" == "200" ]]; then
        echo "Got 200 response, done"
        break
    fi

    echo "Got ${HTTP_STATUS} response, checking again (${CHECK_COUNT}) ..."

    if [[ $CHECK_COUNT -gt $MAX_CHECKS ]]; then
        echo "Reached maximum checks without success."
        exit 1
    fi

    sleep ${SLEEP_TIME}
done

