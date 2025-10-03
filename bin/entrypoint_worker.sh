#!/bin/bash

set -e

# Run codegen
THIS_DIR=$(dirname $(readlink -f $0))
$THIS_DIR/entrypoint_codegen.sh

# Set up isolation
pushd /usr/local/code/faasm >> /dev/null

echo "Setting up cgroup"
./bin/cgroup.sh

echo "Setting up namespaces"
./bin/netns.sh ${MAX_NET_NAMESPACES}

echo "Deplyoment type: ${DEPLOYMENT_TYPE}"
if [[ "$DEPLOYMENT_TYPE" == "compose" ]];
then
    # In a compose deployment, were all workers are co-located in the same host,
    # we need to do some more work to make sure no two Faaslets pin to the
    # same physicial CPU
    echo "Set-up compose-specific env. vars"
    REPLICA_NUM=$( v="$( nslookup "$( hostname -i )" | head -n 1 )"; v="${v##* = }"; v="${v%%.*}"; v="${v##*-}"; v="${v##*_}"; echo ${v})
    export OVERRIDE_FREE_CPU_START=$(( ${REPLICA_NUM}*${OVERRIDE_CPU_COUNT} ))
    echo "Setting override free CPU start to: ${OVERRIDE_FREE_CPU_START}"
fi

popd >> /dev/null

# Continue with normal command
exec "$@"
