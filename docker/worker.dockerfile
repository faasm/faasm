ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}
ARG FAASM_VERSION

# Update code (allows for faster incremental builds without rebuilding base)
WORKDIR /usr/local/code/faasm
RUN git checkout v${FAASM_VERSION}
RUN git submodule update --init

# Build the worker binary
WORKDIR /build/faasm
RUN cmake --build . --target codegen_shared_obj
RUN cmake --build . --target codegen_func
RUN cmake --build . --target pool_runner

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

