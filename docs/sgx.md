# Faasm SGX Support

Faasm provides 
[SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html) support using [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

You can read the latest SGX Linux developer instructions 
[here](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_Developer_Guide.pdf).


## Set-up

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

## Build

To build with SGX support, you need to set `-DFAASM_SGX_SUPPORT=ON`.