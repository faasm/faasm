ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

# Must restate ARG after FROM
ARG FAASM_VERSION

# Get the runtime root from S3
WORKDIR /usr/local/faasm
RUN wget -q https://s3-eu-west-1.amazonaws.com/faasm-misc/faasm-runtime-root-${FAASM_VERSION}.tar.gz
RUN tar --no-same-owner -xf faasm-runtime-root-${FAASM_VERSION}.tar.gz
RUN rm faasm-runtime-root-${FAASM_VERSION}.tar.gz

COPY . /usr/local/code/faasm

# Build the worker binary
WORKDIR /faasm/build
RUN cmake --build . --target worker
RUN cmake --build . --target codegen_shared_obj
RUN cmake --build . --target codegen_func

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_worker.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Create user with dummy uid required by Python
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

ENTRYPOINT ["/entrypoint.sh"]
CMD "/faasm/build/bin/worker"
