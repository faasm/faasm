ARG FAASM_VERSION
ARG FAASM_SGX_PARENT_SUFFIX
FROM ghcr.io/faasm/base${FAASM_SGX_PARENT_SUFFIX}:${FAASM_VERSION}

# Build the worker binary
ARG FAASM_SGX_MODE
RUN cd /usr/local/code/faasm \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && inv dev.cmake \
        --build Release \
        --disable-spinlock \
        --sgx ${FAASM_SGX_MODE} \
    && inv dev.cc codegen_shared_obj \
    && inv dev.cc codegen_func \
    && inv dev.cc pool_runner

WORKDIR /build/faasm

# Install worker-specific deps
RUN apt update && apt install -y dnsutils

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_worker.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Create user with dummy uid required by Python
RUN groupadd -g 1001 faasm
RUN useradd -u 1001 -g 1001 faasm

ENTRYPOINT ["/entrypoint.sh"]
CMD "/build/faasm/bin/pool_runner"

