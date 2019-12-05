ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

# Must restate ARG after FROM
ARG FAASM_VERSION

COPY . /usr/local/code/faasm

# Get the runtime root from S3
WORKDIR /usr/local/faasm
RUN wget -q https://s3-eu-west-1.amazonaws.com/faasm-misc/faasm-runtime-root-${FAASM_VERSION}.tar.gz
RUN tar --no-same-owner -xf faasm-runtime-root-${FAASM_VERSION}.tar.gz
RUN rm faasm-runtime-root-${FAASM_VERSION}.tar.gz

# Build the upload and codegen targets
WORKDIR /faasm/build
RUN cmake --build . --target upload -- -j
RUN cmake --build . --target codegen_shared_obj -- -j
RUN cmake --build . --target codegen_func -- -j

# Put the python no-op function in place
COPY func/python/noop.py /usr/local/faasm/shared_store/pyfuncs/python/noop/function.py

# Set up entrypoint
COPY bin/upload-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD "/faasm/build/bin/upload"