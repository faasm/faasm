FROM shillaker/wasm-toolchain

# Install system deps
RUN apt-get update && apt-get install -y \
    clang-6.0 \
    libboost-dev \
    python-dev \
    python-pip

# Set up clang
RUN ln -s /usr/bin/clang-6.0 /usr/bin/clang
RUN ln -s /usr/bin/clang-cpp-6.0 /usr/bin/clang-cpp
RUN ln -s /usr/bin/clang++-6.0 /usr/bin/clang++
RUN ln -s /usr/bin/llvm-6.0 /usr/bin/llvm

ENV CC "/usr/bin/clang"
ENV CPP "/usr/bin/clang-cpp"
ENV CFLAGS ""
ENV CXX "/usr/bin/clang++"
ENV CXXFLAGS ""

ENV CROSS_COMPILE ""
ENV LD "/usr/bin/ld"
ENV AR "/usr/bin/ar"
ENV AS "/usr/bin/as"
ENV RANLINB "/usr/bin/ranlib"

# Install Python2 deps
RUN pip install multiprocessing

# ----------------------
# Custom musl
# ----------------------

# Note, the steps here are the same as those done in the "musl" step of the
# wasm waterfall build, the script for which can be found here:
# https://github.com/WebAssembly/waterfall/blob/master/src/build.py
WORKDIR /faasm
RUN git clone https://github.com/Shillaker/musl.git
RUN mkdir /faasm/musl-out
WORKDIR /faasm/musl

RUN python2 /faasm/musl/libc.py \
    --clang_dir /toolchain/bin \
    --binaryen_dir /toolchain/bin \
    --sexpr_wasm /toolchain/bin/wat2wasm \
    --musl /faasm/musl \
    --out /faasm/musl-out/libc.a \
    --compile-to-wasm

# Install binaries in place
WORKDIR /faasm/musl-out
RUN /toolchain/bin/llvm-ar rc libm.a
RUN cp libc.a libm.a crt1.o /toolchain/sysroot/lib

# Copy include files
WORKDIR /faasm/musl
RUN cp arch/wasm32/libc.imports /toolchain/sysroot/lib
RUN cp -r include/* /toolchain/sysroot/
RUN cp -r arch/generic/bits/* arch/wasm32/bits* /toolchain/sysroot/include/bits

# ----------------------
# Faasm dependencies
# ----------------------

RUN apt-get install -y python3-dev python3-pip zlib1g-dev
RUN pip3 install invoke requests

COPY . /faasm/code
WORKDIR /faasm/code

# Install all Python deps
RUN pip3 install -r requirements.txt

ENV CC /toolchain/bin/clang
ENV CPP /toolchain/bin/clang-cpp
ENV CFLAGS "--sysroot=/toolchain/sysroot --target=wasm32-unknown-unknown-wasm"
ENV CXX /toolchain/bin/clang++
ENV CXXFLAGS "--sysroot=/toolchain/sysroot --target=wasm32-unknown-unknown-wasm"

ENV CROSS_COMPILE /toolchain/bin/llvm-
ENV LD /toolchain/bin/wasm-ld
ENV AR /toolchain/bin/llvm-ar
ENV AS /toolchain/bin/llvm-as
ENV RANLINB /toolchain/bin/llvm-ranlib

# Install eigen
RUN inv compile-eigen

# Build libcurl
# RUN inv lib curl

# Install libfaasm
RUN inv compile-libfaasm

# Remove code
RUN rm -rf /faasm

WORKDIR /toolchain
CMD "/bin/bash"
