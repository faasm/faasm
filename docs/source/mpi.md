# MPI support

Faasm provides a custom MPI implementation to execute existing unmodified MPI
applications in a serverless context.

The Faasm MPI implementation covers the majority of commonplace MPI
functionality (point-to-point, collective, one-sided, custom datatypes).

## MPI Functions

A number of MPI functions can be found at `func/mpi`. You can compile, upload
and invoke `hellompi` with the following:

```
inv compile mpi hellompi
inv upload mpi hellompi
inv invoke mpi hellompi
```

## Extending the Faasm MPI implementation

The MPI interface declarations live in the [`libfaasmpi`
directory](https://github.com/faasm/cpp/tree/main/libfaasmpi) of
the `cpp` repo. The host interface definitions live
[here](../src/wavm/mpi.cpp).

Any new functions need to be included in the `faasmpi.imports` file in the
`cpp` repo.

## Tuning TCP for MPI

Our MPI implementation is sensitive to the host's TCP/network configuration.
We follow OpenMPI's guidelines on [suggested configuration values](
https://docs.open-mpi.org/en/v5.0.x/tuning-apps/networking/tcp.html).

Setting these values must be done on a per-node basis, and with root privileges,
so rather than setting them programatically, each Faasm deployment should
set them accordingly.

For example, when deploying on `compose` it is enough to run the following
`sysctl` commands on the host:

```bash
sysctl -wq net.core.rmem_max=16777216
sysctl -wq net.core.wmem_max=16777216
sysctl -wq net.ipv4.tcp_rmem='4096 87380 16777216'
sysctl -wq net.ipv4.tcp_wmem='4096 65536 16777216'
sysctl -wq net.core.netdev_max_backlog=30000
sysctl -wq net.core.rmem_default=16777216
sysctl -wq net.core.wmem_default=16777216
sysctl -wq net.ipv4.tcp_mem='16777216 16777216 16777216'
sysctl -wq net.ipv4.route.flush=1
```

similarly, when deploying on a managed Kubernetes service (e.g. AKS) you must
[allow](https://github.com/faasm/experiment-base/blob/main/config/granny_aks_kubelet_config.json)
all `net` `sysctl`s, and set the appropriate values in the [OS](
https://github.com/faasm/experiment-base/blob/main/config/granny_aks_os_config.json).
