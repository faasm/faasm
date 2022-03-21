ARG FAASM_VERSION
FROM faasm/base-sgx:${FAASM_VERSION} AS faasm-base

# Most of this dockerfile is a copy of that in intel/linux-sgx
FROM ubuntu:20.04
RUN apt-get update && apt-get install -y \
    gcc \
    g++ \
    make \
    libcurl4-openssl-dev \
    libprotobuf17 \
    libssl1.1

# Install SDK
WORKDIR /installer
COPY --from=faasm-base /linux-sgx/linux/installer/bin/*.bin ./
COPY --from=faasm-base /opt/intel/sgxsdk /opt/intel/sgxsdk
RUN sh -c 'echo yes | /installer/sgx_linux_x64_sdk*.bin'

# Install PSW
RUN ./sgx_linux_x64_psw*.bin --no-start-aesm

# Build PCKIdRetrievalTool
COPY --from=faasm-base /opt/intel/sgxdcap /opt/intel/sgxdcap
WORKDIR /opt/intel/sgxdcap/tools/PCKRetrievalTool
RUN make

# Set PCCS server info
RUN echo -e "# PCCS server address\nPCCS_URL=https://pccs:8081/sgx/certification/v3/\n# To accept insecure HTTPS cert, set this option to FALSE\nUSE_SECURE_CERT=FALSE" > /etc/sgx_default_qcnl.conf

# Configure AESMD socket
USER aesmd
WORKDIR /opt/intel/sgxpsw/aesm/
ENV LD_LIBRARY_PATH=.
CMD ./aesm_service --no-daemon
