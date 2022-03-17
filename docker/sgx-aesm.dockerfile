ARG FAASM_VERSION
FROM faasm/base-sgx:${FAASM_VERSION} AS faasm-base

# Most of this dockerfile is a copy of that in intel/linux-sgx
FROM ubuntu:18.04
RUN apt-get update && apt-get install -y \
    libcurl4 \
    libprotobuf10 \
    libssl1.1 \
    make \
    module-init-tools

WORKDIR /installer
COPY --from=faasm-base /linux-sgx/linux/installer/bin/*.bin ./
RUN ./sgx_linux_x64_psw*.bin --no-start-aesm
USER aesmd
WORKDIR /opt/intel/sgxpsw/aesm/
ENV LD_LIBRARY_PATH=.
CMD ./aesm_service --no-daemon
