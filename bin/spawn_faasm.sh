#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide a number of workers"
    exit 1
fi

spawn_script="${HOME}/faasm/bench/bin/bench_mem lock"

n_threads=$1
batch_size=2000

n_batches=$(( n_threads / batch_size ))
overshoot=$(( n_threads % batch_size ))
if [ $overshoot -gt 0 ]; then
  n_batches=$(( n_batches + 1 ))
else
  overshoot=$batch_size
fi

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
done

# Wait for things to finish
wait
