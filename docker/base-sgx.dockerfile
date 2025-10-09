ARG FAASM_VERSION
FROM ghcr.io/faasm/base:${FAASM_VERSION}

# Install everything we can from APT repos
RUN apt update && apt install -y --no-install-recommends \
        ca-certificates \
        curl \
        wget \
        gnupg \
        lsb-release \
    && mkdir -p /etc/apt/keyrings \
    && curl -fsSL https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key \
        -o /etc/apt/keyrings/intel-sgx-keyring.asc \
    && echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/intel-sgx-keyring.asc] \
         https://download.01.org/intel-sgx/sgx_repo/ubuntu $(lsb_release -cs) main" \
         > /etc/apt/sources.list.d/intel-sgx.list \
    && apt update \
    # PSW runtime
    && apt install -y --no-install-recommends \
        libsgx-enclave-common \
        libsgx-urts \
        libsgx-launch \
        libsgx-epid \
        libsgx-quote-ex \
        sgx-aesm-service \
        libsgx-aesm-launch-plugin \
        libsgx-aesm-epid-plugin \
        libsgx-aesm-quote-ex-plugin \
    # DCAP runtime and dev headers
    && apt install -y --no-install-recommends \
        libsgx-dcap-ql \
        libsgx-dcap-default-qpl \
        libsgx-dcap-quote-verify \
        libsgx-enclave-common-dev \
        libsgx-dcap-ql-dev \
        libsgx-dcap-default-qpl-dev \
    && rm -rf /var/lib/apt/lists/*

# Instal SGX SDK version from Intel's repository (no APT package)
ARG SGX_SDK_VER=2.26.100.0
ARG SGX_SDK_BIN=sgx_linux_x64_sdk_${SGX_SDK_VER}.bin
ARG SGX_SDK_URL=https://download.01.org/intel-sgx/latest/linux-latest/distro/ubuntu24.04-server/${SGX_SDK_BIN}

RUN wget -qO /tmp/${SGX_SDK_BIN} "${SGX_SDK_URL}" \
    && chmod +x /tmp/${SGX_SDK_BIN} \
    && /tmp/${SGX_SDK_BIN} --prefix /opt/intel \
    # Apply patch to fix the compilation with clang of libraries that consume
    # the SGX SDK.
    && patch -p0 -N --silent < /usr/local/code/faasm/src/enclave/inside/sgxsdk_2.26_libcxx_config.patch \
    # Copy some libraries from the SDK to /usr/lib such that they can be used
    # in HW mode without interfering with the SDK. See: intel/linux-sgx#47
    && cp /opt/intel/sgxsdk/lib64/libsgx_uae_service.so /usr/lib/ \
    && cp /opt/intel/sgxsdk/lib64/libsgx_capable.so /usr/lib/ \
    && rm -f /tmp/${SGX_SDK_BIN}

# Build Faasm with SGX enabled
ARG FAASM_SGX_MODE
RUN cd /usr/local/code/faasm \
    && source venv/bin/activate \
    && git fetch origin \
    && git checkout conan2 \
    && git pull origin conan2 \
    && inv dev.tools \
        --clean \
        --build Release \
        --sgx ${FAASM_SGX_MODE}
