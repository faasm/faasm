ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

COPY . /usr/local/code/faasm

# Build the worker binary
WORKDIR /faasm/build
RUN cmake --build . --target codegen_shared_obj
RUN cmake --build . --target codegen_func
RUN cmake --build . --target pool_runner

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_worker.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Create user with dummy uid required by Python
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

ENTRYPOINT ["/entrypoint.sh"]
CMD "/faasm/build/bin/pool_runner"

