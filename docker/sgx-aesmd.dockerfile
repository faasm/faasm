FROM ubuntu:18.04

RUN apt update && apt install -y \
    gcc \
    git \
    g++ \
    gnupg2 \
    make \
    wget

RUN echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu bionic main' \
    | tee /etc/apt/sources.list.d/intel-sgx.list
RUN wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key \
    | apt-key add -
RUN echo "deb [arch=amd64] https://packages.microsoft.com/ubuntu/18.04/prod bionic main" \
    | tee /etc/apt/sources.list.d/msprod.list
RUN wget -qO - https://packages.microsoft.com/keys/microsoft.asc | apt-key add -


WORKDIR /opt/intel
RUN wget https://download.01.org/intel-sgx/sgx-dcap/1.12.1/linux/distro/ubuntu18.04-server/sgx_linux_x64_sdk_2.15.101.1.bin \
    -O sgx_linux_x64_sdk.bin
RUN chmod a+x sgx_linux_x64_sdk.bin
RUN sh -c 'echo yes | ./sgx_linux_x64_sdk.bin'

RUN apt update
RUN apt install -y \
    libssl-dev \
    libsgx-quote-ex \
    libsgx-enclave-common \
    libsgx-enclave-common-dev \
    libsgx-dcap-ql \
    libsgx-dcap-ql-dev \
    az-dcap-client

WORKDIR /opt/intel/sgx-aesm-service/aesm
ENV LD_LIBRARY_PATH=.
CMD ./aesm_service --no-daemon
