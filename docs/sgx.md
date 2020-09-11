# Faasm SGX Support

Faasm provides 
[SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html) support using [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

You can read the latest SGX Linux developer instructions 
[here](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_Developer_Guide.pdf).

[@J-Heinemann](https://github.com/J-Heinemann) is responsible for the vast majority of the SGX work, 
but some messy merging means he's not fully attributed on certain files. 

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

## Faasm SGX Build

The Faasm SGX build is configured with several CMake options of the form `FAASM_SGX_XXX` in the 
[top-level CMake file](../CMakeLists.txt). 

To use any SGX-related functionality, you must always set `-DFAASM_SGX_SUPPORT=ON`.

To check things are working, you need to:

- Compile with the relevant CMake options switched on
- Build the `sgx_runner` target
- Run `sgx_runner demo hello` to run a hello world function
