ARG FAASM_VERSION
FROM ghcr.io/faasm/base:${FAASM_VERSION:-} AS collector

# Build the upload and codegen targets
RUN cd /usr/local/code/faasm \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && inv dev.cc codegen_shared_obj \
    && inv dev.cc codegen_func \
    && inv dev.cc upload

# Prepare all runtime dependencies
ARG ROOTFS_DIR=/tmp/rootfs
ARG ROOTFS_TAR=/tmp/rootfs.tar
RUN mkdir -p ${ROOTFS_DIR} \
    && /usr/local/code/faasm/bin/collect_runtime_deps.sh \
        --dest ${ROOTFS_DIR} \
        --bin /build/faasm/bin/codegen_shared_obj \
        --bin /build/faasm/bin/codegen_func \
        --bin /build/faasm/bin/upload \
        --emit-tar ${ROOTFS_TAR}

FROM ubuntu:24.04

COPY --from=collector /tmp/rootfs.tar /rootfs.tar
RUN tar -xf /rootfs.tar -C / && rm /rootfs.tar

# Sanity check
RUN ldd /build/faasm/release/bin/upload | awk '/not found/ {nf=1} END{exit nf}'

# Set up entrypoint
RUN ln -s /build/faasm/release/bin /build/faasm/bin
COPY bin/entrypoint_codegen.sh /entrypoint_codegen.sh
COPY bin/entrypoint_upload.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD ["/build/faasm/bin/upload"]
