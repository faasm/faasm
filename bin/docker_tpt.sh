#!/bin/bash

set -e

# NOTE: Bash only does integers therefore we need:
# - sleep duration as a string in seconds (to pass to sleep)
# - duration in millis (as we need to do some arithmetic on it)

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Must provide the delay between requests (SECONDS) and the duration (MILLISECONDS)"
    exit 1
fi

request_delay_s=$1
duration_ms=$2

echo "Starting Docker throughput with delay=${request_delay_s}s and duration=${duration_ms}ms"

# Set up log files
tpt_log_file=/tmp/docker_tpt.log
lat_file=/tmp/docker_lat.log
duration_file=/tmp/docker_duration.log
echo "" > ${tpt_log_file}
echo "" > ${lat_file}
echo "" > ${duration_file}

# Note careful use of date here (https://unix.stackexchange.com/a/123764)
start_ms=$(date +%s%3N)
elapsed_ms=0
request_count=0

this_dir=$(dirname $(readlink -f $0))
inner_script=${this_dir}/docker_tpt_inner.sh

while (( elapsed_ms < duration_ms )); do
  # Launch inner script in background
  ${inner_script} ${lat_file} &

  # Log the launching
  request_count=$(( request_count + 1 ))
  echo "${elapsed_ms} REQUEST ${request_count}" >> ${tpt_log_file}

  # Do the sleep (NOTE SECONDS)
  sleep $request_delay_s

  # Update duration (NOTE INTEGERS ONLY)
  now_ms=$(date +%s%3N)
  elapsed_ms=$(( now_ms - start_ms ))
done

# Wait for finishing processes
wait

# Write final duration
now_ms=$(date +%s%3N)
final_duration_ms=$(( now_ms - start_ms ))
echo "${final_duration_ms} DURATION" >> ${duration_file}

echo "Finished after ${final_duration_ms}ms and made ${request_count} requests."

