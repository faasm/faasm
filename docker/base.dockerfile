# Stages to extract toolchain and sysroot
FROM faasm/cpp-sysroot:0.0.15 as sysroot
FROM faasm/cpython:0.0.7 as cpython

# Import from SGX container
FROM faasm/sgx:0.5.3 as sgx

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm/cpp-root:0.5.1
ARG FAASM_VERSION

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Faasm toolchain
COPY --from=sysroot /usr/local/faasm /usr/local/faasm

# Copy Python outputs 
# TODO - copy these into the toolchain as part of the cpython container build
COPY --from=cpython /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root
COPY --from=cpython \
    /code/faasm-cpython/third-party/cpython/install/wasm/lib/libpython3.8.a \
    /usr/local/faasm/llvm-sysroot/lib/wasm32-wasi/libpython3.8.a
COPY --from=cpython \
    /code/faasm-cpython/third-party/cpython/install/wasm/include/python3.8 \
    /usr/local/faasm/llvm-sysroot/include/python3.8

# Set up SGX SDK
COPY --from=sgx /opt/intel /opt/intel
RUN apt install -y nasm

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

RUN cmake --build . --target tests
RUN cmake --build . --target simple_runner
RUN cmake --build . --target func_runner
RUN cmake --build . --target codegen_func
RUN cmake --build . --target codegen_shared_obj
