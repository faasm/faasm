#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide a number of workers"
    exit 1
fi

# Faasm env
export CGROUP_MODE=off
export NETNS_MODE=off
export LOG_LEVEL=off 
export FS_MODE=on

spawn_script="${HOME}/faasm/bench/bin/bench_mem sleep_long"

n_threads=$1
batch_size=1000

n_batches=$(( n_threads / batch_size ))
overshoot=$(( n_threads % batch_size ))
if [ $overshoot -gt 0 ]; then
  n_batches=$(( n_batches + 1 ))
else
  overshoot=$batch_size
fi

# Bump up ulimit
echo "Attempting to bump up limits..."
echo ""

ulimit -u $(( n_threads + 1000 ))
ulimit -i $(( n_threads + 1000 ))

echo "New limits:"
ulimit -aS

echo ""
echo "Running $n_threads in $n_batches batches"

# Create the lock file
lock_dir=/usr/local/faasm/runtime_root/tmp
mkdir -p $lock_dir
touch $lock_dir/demo.lock

# Make sure logging off
export LOG_LEVEL=off

for (( i=1; i <=n_batches; i++ ))
do
  this_batch=$batch_size

  # Special case for final batch
  if [ $i == $n_batches ]; then
    this_batch=$overshoot
  fi

  # Spawn the processes in the background
  command="$spawn_script $this_batch"
  echo "$command"
  $command &

  sleep 5 
done

# Wait for things to finish
wait
