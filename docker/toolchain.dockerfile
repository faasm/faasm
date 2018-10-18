FROM shillaker/wasm-toolchain

# Build libcurl
COPY . /faasm/code
WORKDIR /faasm/code

RUN inv lib curl

WORKDIR /toolchain

CMD "/bin/bash"