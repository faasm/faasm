# Stages to extract toolchain and sysroot
FROM faasm/sysroot:0.0.10
FROM faasm/cpython:0.0.7

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm/cpp-root:0.4.11
ARG FAASM_VERSION

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Faasm toolchain
COPY --from=0 /usr/local/faasm /usr/local/faasm

# Copy Python outputs 
# TODO - copy these into the toolchain as part of the cpython container build
COPY --from=1 /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root
COPY --from=1 \
    /code/faasm-cpython/third-party/cpython/install/wasm/lib/libpython3.8.a \
    /usr/local/faasm/llvm-sysroot/lib/wasm32-wasi/libpython3.8.a
COPY --from=1 \
    /code/faasm-cpython/third-party/cpython/install/wasm/include/python3.8 \
    /usr/local/faasm/llvm-sysroot/include/python3.8

# Check out code (clean beforehand just in case)
WORKDIR /usr/local/code
RUN rm -rf faasm
RUN git clone \
    -b v${FAASM_VERSION} \
    https://github.com/faasm/faasm
WORKDIR /usr/local/code/faasm

# Set up runtime filesystem
WORKDIR /usr/local/code/faasm/ansible
ENV USER=root
RUN ansible-playbook runtime_fs.yml

# Out of tree build
WORKDIR /build/faasm

# Build the basics here to set up the CMake build
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target libWAVM
RUN cmake --build . --target wamrlib
RUN cmake --build . --target simple_runner
