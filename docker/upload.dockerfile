ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

# Must restate ARG after FROM
ARG FAASM_VERSION

COPY . /usr/local/code/faasm

# Get the runtime root
WORKDIR /usr/local/faasm
RUN wget -q https://github.com/lsds/faasm/releases/download/v${FAASM_VERSION}/faasm-runtime-root-${FAASM_VERSION}.tar.gz
RUN tar --no-same-owner -xf faasm-runtime-root-${FAASM_VERSION}.tar.gz
RUN rm faasm-runtime-root-${FAASM_VERSION}.tar.gz

# Build the upload and codegen targets
WORKDIR /faasm/build
RUN cmake --build . --target upload
RUN cmake --build . --target codegen_shared_obj
RUN cmake --build . --target codegen_func

# Put the python no-op function in place
COPY func/python/noop.py /usr/local/faasm/shared_store/pyfuncs/python/noop/function.py

# Set up entrypoint
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_upload.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD "/faasm/build/bin/upload"