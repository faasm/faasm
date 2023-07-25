ARG FAASM_VERSION
ARG FAASM_SGX_PARENT_SUFFIX
FROM faasm.azurecr.io/base${FAASM_SGX_PARENT_SUFFIX}:${FAASM_VERSION}

# Build the worker binary
ARG FAASM_SGX_MODE
# FIXME: remove me
ARG FAASM_VERSION
RUN cd /usr/local/code/faasm \
    # FIXME: temporarily re-build the worker from the latest commit in the tag
    && git checkout v${FAASM_VERSION} && git pull origin v${FAASM_VERSION} && git submodule update \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && inv -r faasmcli/faasmcli dev.cmake --build Release --sgx ${FAASM_SGX_MODE} \
    && inv -r faasmcli/faasmcli dev.cc codegen_shared_obj \
    && inv -r faasmcli/faasmcli dev.cc codegen_func \
    && inv -r faasmcli/faasmcli dev.cc pool_runner

WORKDIR /build/faasm

# Install hoststats
RUN pip3 install hoststats==0.1.0

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_worker.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Create user with dummy uid required by Python
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

ENTRYPOINT ["/entrypoint.sh"]
CMD "/build/faasm/bin/pool_runner"

