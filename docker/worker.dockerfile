FROM faasm/base

# Get the runtime root from S3
WORKDIR /usr/local/faasm
RUN wget -q https://s3-eu-west-1.amazonaws.com/faasm-misc/faasm_runtime_root.tar.gz
RUN tar --no-same-owner -xf faasm_runtime_root.tar.gz
RUN rm faasm_runtime_root.tar.gz

COPY . /usr/local/code/faasm

# Build the worker binary
WORKDIR /faasm/build
RUN cmake --build . --target worker -- -j

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/worker-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Create user with dummy uid required by Python
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

ENTRYPOINT ["/entrypoint.sh"]
CMD "/faasm/build/bin/worker"
