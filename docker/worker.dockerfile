ARG FAASM_VERSION
ARG FAASM_SGX_PARENT_SUFFIX
FROM ghcr.io/faasm/base${FAASM_SGX_PARENT_SUFFIX}:${FAASM_VERSION} AS collector

# Build the worker binary
ARG FAASM_SGX_MODE
RUN cd /usr/local/code/faasm \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && inv dev.cmake \
        --build Release \
        --disable-spinlock \
        --sgx ${FAASM_SGX_MODE} \
    && inv dev.cc codegen_shared_obj \
    && inv dev.cc codegen_func \
    && inv dev.cc pool_runner

# Prepare all runtime dependencies
ARG ROOTFS_DIR=/tmp/rootfs
ARG ROOTFS_TAR=/tmp/rootfs.tar
RUN mkdir -p ${ROOTFS_DIR} \
    && /usr/local/code/faasm/bin/collect_runtime_deps.sh \
        --dest ${ROOTFS_DIR} \
        --bin /build/faasm/bin/codegen_shared_obj \
        --bin /build/faasm/bin/codegen_func \
        --bin /build/faasm/bin/pool_runner \
        --emit-tar ${ROOTFS_TAR}

FROM ubuntu:24.04

COPY --from=collector /tmp/rootfs.tar /rootfs.tar
RUN tar -xf /rootfs.tar -C / && rm /rootfs.tar

# Install worker-specific deps
RUN apt update && apt install -y dnsutils

# Sanity check
RUN ldd /build/faasm/release/bin/upload | awk '/not found/ {nf=1} END{exit nf}'

# Copy runtime root and runtime dependencies.
ARG FAASM_SOURCE=/usr/local/code/faasm
COPY --from=collector /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root
COPY --from=collector ${FAASM_SOURCE}/bin/entrypoint_codegen.sh ${FAASM_SOURCE}/bin/entrypoint_codegen.sh
COPY --from=collector ${FAASM_SOURCE}/bin/entrypoint_worker.sh ${FAASM_SOURCE}/bin/entrypoint_worker.sh
COPY ./bin/cgroup.sh ${FAASM_SOURCE}/bin/cgroup.sh
COPY --from=collector ${FAASM_SOURCE}/bin/netns.sh ${FAASM_SOURCE}/bin/netns.sh

# Set up entrypoint (for cgroups, namespaces etc.)
RUN ln -s /build/faasm/release/bin /build/faasm/bin

# Create user with dummy uid required by Python
RUN groupadd -g 1001 faasm
RUN useradd -u 1001 -g 1001 faasm

ENTRYPOINT ["/usr/local/code/faasm/bin/entrypoint_worker.sh"]
CMD ["/build/faasm/bin/pool_runner"]

