FROM ubuntu:20.04

# System deps
RUN apt-get update
RUN apt-get install -y \
    ocaml \
    ocamlbuild

WORKDIR /linux-sgx
RUN git clone https://github.com/intel/linux-sgx.git
RUN make preparation
RUN make sdk_install_pkg_no_mitigation

# Install SGX SDK
# TODO - should we pin to a specific release?
WORKDIR /opt/intel
RUN sh -c 'echo yes | /inux-sgx/linux/installer/bin/sgx_linux_x64_sdk_*.bin'

# Install PSW
WORKDIR /linux-sgx
RUN make psw_install_pkg

# # Download SDK installer
# WORKDIR /sgx
# RUN wget https://download.01.org/intel-sgx/sgx-linux/2.12/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.12.100.3.bin
#
# # Install the SDK
# RUN chmod +x sgx_linux_x64_sdk_2.12.100.3.bin
# RUN ./sgx_linux_x64_sdk_2.12.100.3.bin --prefix /opt/intel
