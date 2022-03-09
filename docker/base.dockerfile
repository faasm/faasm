# Stage to extract Python runtime files
FROM faasm/cpython:0.1.0 as python

# Import from SGX container
FROM faasm/sgx:0.8.1 as sgx

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm/cpp-root:0.8.0
ARG FAASM_VERSION

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Python runtime libraries
COPY --from=python /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root

# Set up SGX SDK, and install the SDK and PSW libraries
COPY --from=sgx /opt/intel /opt/intel
COPY --from=sgx /linux-sgx/linux/installer/bin/ /tmp/linux-sgx
WORKDIR /opt/intel
RUN sh -c 'echo yes | /tmp/linux-sgx/sgx_linux_x64_sdk_2.15.101.1.bin'
RUN sh -c 'echo yes | /tmp/linux-sgx/sgx_linux_x64_psw_2.15.101.1.bin --no-start-aesm'
RUN cp /opt/intel/sgxsdk/lib64/libsgx_capable.so /usr/lib

# Check out code (clean beforehand just in case)
WORKDIR /usr/local/code
RUN rm -rf faasm
RUN git clone \
    -b v${FAASM_VERSION} \
    https://github.com/faasm/faasm
WORKDIR /usr/local/code/faasm

RUN git submodule update --init

# Set up runtime filesystem
RUN mkdir -p /usr/local/faasm/runtime_root/etc
RUN cp deploy/conf/hosts /usr/local/faasm/runtime_root/etc/
RUN cp deploy/conf/resolv.conf /usr/local/faasm/runtime_root/etc/
RUN cp deploy/conf/passwd /usr/local/faasm/runtime_root/etc/
RUN mkdir -p /usr/local/faasm/runtime_root/tmp
RUN mkdir -p /usr/local/faasm/runtime_root/share

# Out of tree build
WORKDIR /build/faasm

# Build the basics here to set up the CMake build
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-13 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-13 \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAASM_SGX_MODE=Simulation \
    /usr/local/code/faasm

RUN cmake --build . --target tests
RUN cmake --build . --target func_runner
RUN cmake --build . --target codegen_func
RUN cmake --build . --target codegen_shared_obj
