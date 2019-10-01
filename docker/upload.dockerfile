FROM faasm/base

COPY . /usr/local/code/faasm

# Get the runtime root from S3
WORKDIR /usr/local/faasm
RUN wget -q https://s3-eu-west-1.amazonaws.com/faasm-misc/faasm_runtime_root.tar.gz
RUN tar --no-same-owner -xf faasm_runtime_root.tar.gz
RUN rm faasm_runtime_root.tar.gz

# Build the upload and codegen targets
WORKDIR /faasm/build
RUN cmake --build . --target upload -- -j
RUN cmake --build . --target codegen_shared_obj -- -j
RUN cmake --build . --target codegen_func -- -j

# Set up entrypoint
COPY bin/upload-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD "/faasm/build/bin/upload"