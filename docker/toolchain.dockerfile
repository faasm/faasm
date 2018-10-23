FROM shillaker/wasm-toolchain

# Install system deps
RUN apt-get update && apt-get install -y \
    libboost-dev \
    python3-dev \
    python3-pip
    
RUN pip3 install invoke requests

COPY . /faasm/code
WORKDIR /faasm/code

# Install eigen
RUN inv lib eigen

# Build libcurl
RUN inv lib curl

WORKDIR /toolchain
CMD "/bin/bash"
