FROM shillaker/wasm-toolchain

# Install system deps
RUN apt-get update && apt-get install -y \
    libboost-dev \
    python3-dev \
    python3-pip

RUN pip3 install invoke requests

# ----------------------
# Custom musl
# ----------------------

WORKDIR /faasm
RUN git clone git@github.com:Shillaker/musl.git
WORKDIR /faasm/musl-work

RUN python /faasm/musl/libc.py \
    --clang_dir /toolchain/bin \
    --binaryen_dir /toolchain/bin \
#    --sexpir_wasm /toolchain/bin/wat2wasm \
    --out libc.a \
    --musl /faasm/musl \
    --compile-to-wasm

RUN /toolchain/bin/llvm-ar rc libm.a

# Copy into place in sysroot
RUN cp libc.a libm.a crt1.o arch/wasm32/libc.imports /toolchain/sysroot/lib

# Copy include files
WORKDIR /faasm/musl
RUN cp -r include/* /toolchain/sysroot/
RUN cp -r arch/generic/bits/* arch/wasm32/bits* /toolchain/sysroot/include/bits

# ----------------------
# Faasm dependencies
# ----------------------

COPY . /faasm/code
WORKDIR /faasm/code

# Install eigen
RUN inv lib eigen

# Build libcurl
RUN inv lib curl

# Remove code
RUN rm -rf /faasm

WORKDIR /toolchain
CMD "/bin/bash"
