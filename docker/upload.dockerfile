ARG FAASM_VERSION
FROM ghcr.io/faasm/base:${FAASM_VERSION}

# Build the upload and codegen targets
RUN cd /usr/local/code/faasm \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && inv dev.cc codegen_shared_obj \
    && inv dev.cc codegen_func \
    && inv dev.cc upload

# Install hoststats
RUN pip3 install hoststats==0.1.0

# Set up entrypoint
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_upload.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD "/build/faasm/bin/upload"
