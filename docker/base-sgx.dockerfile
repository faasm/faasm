ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install  -y \
    debhelper \
    lsb-release \
    ocaml \
    ocamlbuild \
    python

# We must install protobuf manually, as a static library and PIC. Otherwise,
# attestation will fail at runtime with a hard to debug segmentation fault when
# dlopen-ing some sgx-related libraries. The problem arises when these libraries
# link with protobuf as a shared library, as reported in protobuf#206
RUN git clone -b v3.17.0 \
        https://github.com/protocolbuffers/protobuf.git \
        /tmp/protobuf \
    && mkdir -p /tmp/protobuf/build \
    && cd /tmp/protobuf/build \
    && cmake \
        -Dprotobuf_BUILD_SHARED_LIBS=OFF \
        -Dprotobuf_BUILD_TESTS=OFF \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        ../cmake/ \
    && make \
    && make install

# Build and install SGX SDK and PSW
ARG SGX_SDK_VERSION=2.15.1
# 09/03/2022 - As part of the preparation step, we download pre-built binaries
# from Intel's official repositories. There does not seem to be a clear way
# to specify which version to download. We pin to code version 2.15.101.1. It
# may happen that, at some point, the image build fails because the preparation
# script points to out-of-date links. In that case we will have to clone from a
# more recent tag.
RUN git clone -b sgx_${SGX_SDK_VERSION} https://github.com/intel/linux-sgx.git \
    && cd /linux-sgx \
    && make preparation \
    # Build SDK and install package
    && make sdk_install_pkg_no_mitigation \
    && mkdir -p /opt/intel \
    && cd /opt/intel \
    && sh -c "echo yes | /linux-sgx/linux/installer/bin/sgx_linux_x64_sdk_${SGX_SDK_VERSION}01.1.bin" \
    && cd /linux-sgx \
    && make psw_install_pkg \
    && cd /opt/intel \
    && sh -c "echo yes | /linux-sgx/linux/installer/bin/sgx_linux_x64_psw_${SGX_SDK_VERSION}01.1.bin --no-start-aesm" \
    # In hardware builds we don't want to link against any library in the SDK, thus
    # we copy this library to detect sgx into `/usr/lib`. See this related issue:
    # https://github.com/intel/linux-sgx/issues/47
    && cp /opt/intel/sgxsdk/lib64/libsgx_capable.so /usr/lib

# Install SGX DCAP
ARG DCAP_VERSION=1.12.1
RUN git clone -b DCAP_${DCAP_VERSION} \
        https://github.com/intel/SGXDataCenterAttestationPrimitives.git \
        /opt/intel/sgxdcap \
    &&  cd /opt/intel/sgxdcap/QuoteGeneration \
    && ./download_prebuilt.sh \
    && make \
    # We need this soft-link as otherwise the runtime linking fails
    && ln -s /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_dcap_ql.so \
        /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_dcap_ql.so.1 \
    # Install manually the libraries under `/usr/lib` for a lack of a `make install`
    # recipe
    && cp /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_dcap_ql.so* \
        /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_pce_logic.so \
        /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_qe3_logic.so \
        /usr/lib/

# Build Faasm with SGX enabled
ARG FAASM_SGX_MODE
RUN cd /build/faasm \
    && cmake \
        -GNinja \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++-13 \
        -DCMAKE_C_COMPILER=/usr/bin/clang-13 \
        -DCMAKE_BUILD_TYPE=Release \
        -DFAASM_SGX_MODE=${FAASM_SGX_MODE} \
        /usr/local/code/faasm \
     && cmake --build . --target tests \
     && cmake --build . --target func_runner \
     && cmake --build . --target codegen_func \
     && cmake --build . --target codegen_shared_obj
