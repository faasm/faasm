FROM ubuntu:20.04

# System deps
RUN apt-get update
RUN apt-get install -y \
    dkms \
    nasm \
    wget

# Download SDK installer
WORKDIR /sgx
RUN wget https://download.01.org/intel-sgx/sgx-linux/2.12/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.12.100.3.bin

# Install the SDK
RUN chmod +x sgx_linux_x64_sdk_2.12.100.3.bin
RUN ./sgx_linux_x64_sdk_2.12.100.3.bin --prefix /opt/intel
