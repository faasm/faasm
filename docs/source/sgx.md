# Faasm SGX Support

Faasm provides
[SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html)
support using [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

## Quick start

To configure SGX, we must build the code with the desired SGX flavour: disabled
(no SGX functionality), simulation (default), or hardware.

First, access an SGX-enabled docker-based CLI:

```bash
./bin/cli.sh faasm-sgx
```

```bash
inv dev.cmake --sgx Disabled|Simulation|Hardware
```

Note that to run in hardware mode, you must have an SGX-capable CPU where
SGX has been enabled. To check you may run:

```bash
inv dev.cc detect_sgx
inv sgx.check
```

Then, to run functions using SGX compile them as usual, and set the `WASM_VM`
environment variable to "sgx".

```bash
# Start development cluster, and log into the cpp container
./bin/cli.sh cpp

# Compile the demo function
inv func demo hello

# Exit the cpp container, and log into the CLI one
exit
./bin/cli.sh faasm

# Set SGX as our execution mode of choice, and operate as usual
export WASM_VM="sgx"

# Generate machine code
inv codegen demo hello

# Run the code
inv run demo hello
```

To run SGX in an Azure kubernetes cluster, see the relevant repositories:
[experiment-base](https://github.com/faasm/experiment-base) and
[experiment-sgx](https://github.com/faasm/experiment-sgx).

## Update SGX SDK and PSW version

We use SGX SDK and PSW version [`2.15.1`](https://github.com/intel/linux-sgx/tree/sgx_2.15.1)
as defined in [`sgx.dockerfile`](https://github.com/faasm/faasm/blob/main/docker/sgx.dockerfile).
If you want to upgrade the version, change it there, and create a new docker
image using:

```bash
inv docker.build -c base-sgx -c base-sgx-sim --nocache --push
```

The image will be tagged with the current code version, which you will have to
amend in [`base.dockerfile`](https://github.com/faasm/faasm/blob/be3f2f73ec1120047ddabd1a50629d1b075023e6/docker/base.dockerfile#L5).
