FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# System deps
RUN apt-get update
RUN apt-get install  -y \
    autoconf \
    automake \
    build-essential \
    cmake \
    curl \
    debhelper \
    git \
    libcurl4-openssl-dev \
    libprotobuf-dev \
    libssl-dev \
    libtool \
    lsb-release \
    ocaml \
    ocamlbuild \
    protobuf-compiler \
    python \
    wget \
    unzip

# 09/03/2022 - As part of the preparation step, we download pre-built binaries
# from Intel's official repositories. There does not seem to be a clear way
# to specify which version to download. As this image is not re-built often,
# we pin to version 2.15.101.1. It may happen that, at some point, the image
# build fails because the preparation script points to out-of-date links. In
# that case we will have to clone from a more recent tag.
RUN git clone -b sgx_2.15.1 https://github.com/intel/linux-sgx.git /linux-sgx
WORKDIR /linux-sgx
RUN make preparation

# Build SDK and install package. In order to build PSW, we need to also install
# the package.
RUN make sdk_install_pkg_no_mitigation
WORKDIR /opt/intel
RUN sh -c 'echo yes | /linux-sgx/linux/installer/bin/sgx_linux_x64_sdk_2.15.101.1.bin'

# Build PSW and prepare install package
WORKDIR /linux-sgx
RUN make psw_install_pkg
WORKDIR /opt/intel
