# SGX support

Faasm provides
[SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html)
support using [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

> [!WARNING]
> We restrict the use of WAMR + SGX to SGX v2 **only**. This is to ensure we
> can use SGX's [dynamic memory management features](https://github.com/intel/sgx-emm).
> This means that, to run in HW mode, you need: (i) an Intel IceLake server
> (or newer), and (ii) a host kernel > 6.0 (EDMM was upstreamed with the
> in-kernel SGX driver then). However, this configuration only works for HW
> mode, as the EDMM symbols are not in the SIM mode libraries. We test it
> as part of our GHA run in HW mode. For the SIM mode tests we use an older
> SGX configuration.

## Quick start

To configure SGX, we must build the code with the desired SGX flavour: disabled
(no SGX functionality), simulation (default), or hardware.

First, access an SGX-enabled docker-based CLI:

```bash
WASM_VM=sgx-sim faasmctl cli.faasm
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
faasmctl cli.cpp

# Compile the demo function
inv func demo hello

# Exit the cpp container, and log into the CLI one
exit
faasmctl cli.faasm

# Set SGX as our execution mode of choice, and operate as usual
export WASM_VM="sgx"

# Generate machine code
inv codegen demo hello

# Run the code
inv run demo hello
```

To run a development cluster with SGX run:

```bash
WASM_VM=sgx(-sim) faasmctl deploy.compose --mount-source .
```

To run SGX in an Azure kubernetes cluster, see the relevant repositories:
[experiment-base](https://github.com/faasm/experiment-base) and
[experiment-tless](https://github.com/faasm/experiment-tless).

## Remote Attestation

Attesting an SGX enclave consists of two steps:
1. Generating a quote for the attested enclave.
2. Validate the enclave's quote with a remote attestation service.

Faasm's implementation of remote attestation uses Microsoft Azure's attestation
service.

### Quote Generation

An enclave quote is an enclave's report signed by the platform's Quoting Enclave
(QE). An enclave report is a measure of the enclave's memory content signed to
the enclave's identity. The QE is a special-purpose Intel-provisioned singleton
enclave running in all SGX-enabled and SGX-capable platforms. This means we can
only generate a quote in hardware mode.

Additionally, quote generation can happen in two modes `in-proc` and
`out-of-proc`. The latter is the preferred one for performance and memory
consumption, and it is also the required mode for integration with AKS. To
generate a quote in `out-of-proc` mode, we need to run a service that wraps the
QE and signs reports on demand. This service is called AESM, and it listens for
incoming requests in a UNIX socket. In a development cluster, we deploy this
service as part of our container mesh. In an AKS cluster this service is
provided by the AKS runtime.

As an artifact of the quote generation process we have a byte array with the
enclave's quote. We persist useful information, together with the quote, in a
helper class: `sgx::EnclaveInfo`.

### Quote Validation

To validate an enclave's quote, we need to send a request to attest an SGX
enclave to an Azure Attestation Provider,
[link to the API](https://docs.microsoft.com/en-us/rest/api/attestation/attestation/attest-sgx-enclave).
If the request goes through (i.e. the enclave's quote passes the policy loaded
in the attestation provider) we receive a JWT. Before deeming the attestation
as valid, we check the integrity of the actual JWT.

## Update SGX SDK and PSW version

We use SGX SDK and PSW version [`2.18.1`](https://github.com/intel/linux-sgx/tree/sgx_2.18.1)
as defined in [`base-sgx.dockerfile`](https://github.com/faasm/faasm/blob/main/docker/base-sgx.dockerfile).
If you want to upgrade the version, change it there, and create a new docker
image using:

```bash
inv docker.build -c base-sgx -c base-sgx-sim --nocache --push
```

The image will be tagged with the current code version, which you will have to
amend in [`base.dockerfile`](https://github.com/faasm/faasm/blob/be3f2f73ec1120047ddabd1a50629d1b075023e6/docker/base.dockerfile#L5).
