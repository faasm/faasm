FROM shillaker/wasm-toolchain

COPY . /faasm/code
WORKDIR /faasm/code

# Install system deps
RUN apt-get install -y vim \
    libboost-dev

# Build libcurl
RUN inv lib curl

WORKDIR /toolchain
CMD "/bin/bash"
