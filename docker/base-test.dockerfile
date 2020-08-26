ARG FAASM_VERSION
FROM faasm/cpp-root:0.4.0
ARG FAASM_VERSION

# ------------------------------------------------------
# This image should contain everything needed for tests
# that doesn't need to rebuilt for every run.
#
# Note that it must be based on Git to work with the CI
# (rather than a COPY command). It can be slightly out of
# date, as the build will update the code and rebuild.
# ------------------------------------------------------

RUN apt-get update
RUN apt-get install -y apt-transport-https
RUN apt-get install -y \
    git \
    ssh \
    tar \
    gzip \
    ca-certificates \
    libcairo2-dev \
    python3-dev \
    python3-venv \
    libtinfo5

# User and permissions
RUN mkdir /usr/local/faasm
RUN chown -R root:root /usr/local/faasm
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

# Set up requests
RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

# Install other python deps
COPY faasmcli/requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Check out code
WORKDIR /usr/local/code
RUN rm -r faasm
RUN git clone https://github.com/lsds/faasm

# Check out latest tag
WORKDIR /usr/local/code/faasm
RUN git fetch --all
RUN git checkout v${FAASM_VERSION}

# Submodules
RUN git submodule update --init third-party/eigen
RUN git submodule update --init third-party/faabric
RUN git submodule update --init third-party/WAVM
RUN git submodule update --init third-party/wamr
RUN git submodule update --init third-party/faasm-toolchain

# Set up deps
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook catch.yml
RUN ansible-playbook eigen.yml

# Install the CLI
WORKDIR /usr/local/code/faasm
RUN pip3 install -e faasmcli/

# Download the toolchain (needed to build libs)
RUN inv -r faasmcli/faasmcli toolchain.download-toolchain

# Create out of tree build
WORKDIR /usr/local/code/faasm/build
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    -DCMAKE_BUILD_TYPE=Release \
    ..

# Run builds
RUN cmake --build . --target codegen_shared_obj
RUN cmake --build . --target codegen_func
RUN cmake --build . --target tests

# Note - all codegen must be left to the build environment

# Clear out
RUN rm -rf /tmp/*

