# Faasm SGX Support

Faasm provides
[SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html)
support using [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

## Quick start

SGX support is currently only available for development clusters.
Note that the following will run in simulation mode.

```bash
# Start development cluster, and log into the cpp container
./bin/cli.sh cpp

# Compile the demo function
inv func demo hello

# Exit the cpp container, and log into the CLI one
exit
./bin/cli.sh faasm

# Generate machine code for SGX
inv codegen demo hello --wamr --sgx

# Run the code
inv run demo hello --wamr --sgx
```

## SGX Set-up

_Simulation Mode_

You'll need to install SGX. You can see how this is done by looking at the
[`faasm/sgx` dockerfile](../docker/sgx.dockerfile).

Simulation mode is the default for Faasm builds.

_Hardware mode_

Hardware Mode requires more SGX packages than Simulation Mode (e.g. SGX driver,
SGX LE etc.). The drive and prebuilt binaries can be downloaded from [the Intel
website](https://download.01.org/intel-sgx/sgx-linux/2.12/distro/ubuntu20.04-server/).

To run in Hardware Mode, you need to set `FAASM_SGX_SIM_MODE=OFF` (obviously
this is only possible with an SGX-enabled CPU).

## Building Faasm with SGX enabled

The Faasm build will automatically detect some properties of your SGX
installation with [SGX-CMake](https://github.com/xzhangxa/SGX-CMake).

High-level Faasm SGX support is configured with CMake options in the [main
`CMakeLists.txt`](../CMakeLists.txt).

Low-level SGX-related customisation is found in [the SGX-specific
`CMakeLists.txt`](../src/sgx/CMakeLists.txt).

### General Options

- `FAASM_SGX_SIM_MODE` - Specifies the SGX execution mode whether the Simulation
  or Hardware mode is used. Enabling this option chooses the Simulation mode.
  Default is on.
