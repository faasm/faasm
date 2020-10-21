# Stages to extract toolchain and sysroot
FROM faasm/sysroot:0.0.10
FROM faasm/cpython:0.0.6 

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm/cpp-root:0.4.11
ARG FAASM_VERSION

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Faasm local dir
COPY --from=0 /usr/local/faasm /usr/local/faasm
COPY --from=1 /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root

# Check out code
WORKDIR /usr/local/code
RUN rm -rf faasm
RUN git clone \
    -b v${FAASM_VERSION} \
    https://github.com/faasm/faasm
WORKDIR /usr/local/code/faasm
RUN git submodule update --init

# Out of tree build
WORKDIR /build/faasm

# Build WAVM and WAMR to avoid repetition in other dockerfiles
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target libWAVM
RUN cmake --build . --target wamrlib
