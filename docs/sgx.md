# Faasm SGX Support

Faasm provides
[SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html)
support using [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

You can read the latest SGX Linux developer instructions 
[here](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_Developer_Guide.pdf).

[@J-Heinemann](https://github.com/J-Heinemann) is responsible for the vast
majority of the SGX work, but some messy merging means he's not fully attributed
on certain files. 

## Faasm-SGX Requirements

_Development_

Because Intel SGX offers a so-called "Simulation Mode", there is no need to use
CPU with SGX capability.  Just set ` option(FAASM_SGX_SIM_MODE "SGX sim mode
flag" ON)` or use `-DFAASM_SGX_SIM_MODE=ON` as additional CMake flag.

Please be aware that the hardware-enforced protection mechanism of SGX is NOT
active in `-DFAASM_SGX_SIM_MODE=ON`!

_Active SGX protection_

The second execution mode of SGX is called "Hardware Mode". In this mode, the
hardware-enforced protection mechanism is enabled and requires an Intel CPU with
SGX capability.  Otherwise Faasm-SGX is unable to start and aborts with a
"missing-sgx" error message.

## SGX Set-up

To set up your machine for SGX you must run the following:

```
# Download installers
cd ansible
ansible-playbook sgx.yml --ask-become-pass

# Execute driver installer
sudo /tmp/sgx_linux_x64_driver_2.6.0_602374c.bin
```

To install the SDK you must specify which directory you want it in, to do this, 
just run in the right place:

```
# Execute SDK installer  
cd /opt/intel
sudo /tmp/sgx_linux_x64_sdk_2.10.100.2.bin

# Enter "yes" 
```

_Active SGX protection_

Hardware Mode requires more SGX packages (e.g. SGX driver, SGX LE etc.). The
[driver](https://download.01.org/intel-sgx/sgx-linux/2.11/distro/ubuntu18.04-server/)
and all prebuilt binaries can be downloaded
[here](https://download.01.org/intel-sgx/sgx-linux/2.11/distro/ubuntu18.04-server/debian_pkgs/utils/).

## Faasm-SGX Build

The Faasm-SGX build is configured with several CMake options.  General options
(e.g. SGX execution mode, enabling extensions etc) are located
[here](../CMakeLists.txt).  Very specific options regarding Faasm-SGX (e.g.
compiler-& linker flags, Faasm-SGX TCS number etc) are located
[here](../src/sgx/CMakeLists.txt)

### General Options

- `FAASM_SGX_SUPPORT` - Enables or disables the SGX extension. Default is on.
- `FAASM_SGX_SIM_MODE` - Specifies the SGX execution mode whether the Simulation
  or Hardware mode is used.  Enabling this option chooses the Simulation mode.
  Default is on.
- `FAASM_SGX_WAMR_AOT_MODE` - Activates the Ahead of Time execution mode of
  WAMR.  If enabled, AoT is used. Otherwise the interpreter mode of WAMR is
  used. Default is on.
- `FAASM_SGX_ATTESTATION` - Enables the Attestation extension. If this option is
  enabled, please set `#define FAASM_SGX_ATTESTATION 1` in
  [faasm_sgx_enclave.edl](../src/sgx/faasm_sgx_enclave.edl).  Default is off.
- `FAASM_SGX_WHITELISTING` - Enables the Whitelisting extension. This option is 
  only available if `FAASM_SGX_WAMR_AOT_MODE=OFF`.  Default is off.
- `FAASM_SGX_XRA` - Enables the eXtended Remote Attestation mechanism. Default 
  is off.

### Low-Level Options

- `FAASM_SGX_DEBUG`- Prints debug information messages.  Useful for development
  but slows down the execution.  Default is 1
- `FAASM_SGX_INIT_TCS_SLOTS` - Specifies the amount of Faasm-SGX TCS slots after
  initialization.  Its recommended (but not necessary) to use the same number as
  available SGX TCS.  Default is 2.
- `FAASM_SGX_ENCLAVE_PATH` - Specifies the path to the enclave. Default is
  `sgx_wamr_enclave.sign.so`
- `FAASM_SGX_WAMR_BUILDIN_LIBC` - This option enables the WAMR built-in libc.
  Default is 1 (enabled).
- `FAASM_SGX_WAMR_WASI_LIBC` - This option enables WASI support. If this option 
  is enabled, please set `#define FAASM_SGX_WAMR_WASI_LIBC 1` in 
  [faasm_sgx_enclave.edl](../src/sgx/faasm_sgx_enclave.edl). Default is 0 
  (disabled).
- `SGX_DEBUG_MODE` - This option enables or disables the SGX debug mode. 
  Default is 1 (enabled).
