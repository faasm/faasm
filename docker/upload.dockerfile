ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

COPY . /usr/local/code/faasm

# Build the upload and codegen targets
WORKDIR /build/faasm
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

CMD "/build/faasm/bin/upload"
