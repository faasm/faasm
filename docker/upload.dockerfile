ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

# Build the upload and codegen targets
RUN cd /build/faasm \
    && cmake --build . --target upload \
    && cmake --build . --target codegen_shared_obj \
    && cmake --build . --target codegen_func

# Install hoststats
RUN pip3 install hoststats==0.1.0

# Set up entrypoint
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_upload.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD "/build/faasm/bin/upload"
