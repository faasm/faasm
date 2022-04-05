ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

# ----------------------------
# Install SGX SDK and PSW
# ----------------------------

ENV DEBIAN_FRONTEND=noninteractive

# TODO - can we use our installed protobuf?
RUN apt-get update
RUN apt-get install  -y \
    debhelper \
    lsb-release \
    ocaml \
    ocamlbuild \
    python

# We must install protobuf manually, as a static library and PIC. Otherwise,
# attestation will fail at runtime with a hard to debug segmentation fault when
# dlopen-ing some sgx-related libraries. The problem arises when these libraries
# link with protobuf as a shared library, as reported in GH issues:
WORKDIR /tmp
RUN git clone -b v3.17.0 https://github.com/protocolbuffers/protobuf.git
WORKDIR /tmp/protobuf/build
RUN cmake \
    -Dprotobuf_BUILD_SHARED_LIBS=OFF \
    -Dprotobuf_BUILD_TESTS=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    ../cmake/
RUN make
RUN make install

# 09/03/2022 - As part of the preparation step, we download pre-built binaries
# from Intel's official repositories. There does not seem to be a clear way
# to specify which version to download. We pin to code version 2.15.101.1. It
# may happen that, at some point, the image build fails because the preparation
# script points to out-of-date links. In that case we will have to clone from a
# more recent tag.
RUN git clone -b sgx_2.15.1 https://github.com/intel/linux-sgx.git /linux-sgx
WORKDIR /linux-sgx
RUN make preparation

# Build SDK and install package
RUN make sdk_install_pkg_no_mitigation
WORKDIR /opt/intel
RUN sh -c 'echo yes | /linux-sgx/linux/installer/bin/sgx_linux_x64_sdk_2.15.101.1.bin'

# Build PSW and install package
WORKDIR /linux-sgx
RUN make psw_install_pkg
WORKDIR /opt/intel
RUN sh -c 'echo yes | /linux-sgx/linux/installer/bin/sgx_linux_x64_psw_2.15.101.1.bin --no-start-aesm'
# In hardware builds we don't want to link against any library in the SDK, thus
# we copy this library to detect sgx into `/usr/lib`. See this related issue:
# https://github.com/intel/linux-sgx/issues/47
RUN cp /opt/intel/sgxsdk/lib64/libsgx_capable.so /usr/lib

# ----------------------------
# Install SGX DCAP
# ----------------------------

RUN git clone -b DCAP_1.12.1 \
    https://github.com/intel/SGXDataCenterAttestationPrimitives.git \
    /opt/intel/sgxdcap
WORKDIR /opt/intel/sgxdcap/QuoteGeneration
RUN ./download_prebuilt.sh
RUN make
# We need this soft-link as otherwise the runtime linking fails
RUN ln -s /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_dcap_ql.so \
    /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_dcap_ql.so.1
# Install manually the libraries under `/usr/lib` for a lack of a `make install`
# recipe
RUN cp /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_dcap_ql.so* \
    /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_pce_logic.so \
    /opt/intel/sgxdcap/QuoteGeneration/build/linux/libsgx_qe3_logic.so \
    /usr/lib/

# ----------------------------
# Build Fassm
# ----------------------------

WORKDIR /build/faasm
ARG FAASM_SGX_MODE
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-13 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-13 \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAASM_SGX_MODE=${FAASM_SGX_MODE} \
    /usr/local/code/faasm

RUN cmake --build . --target tests
RUN cmake --build . --target func_runner
RUN cmake --build . --target codegen_func
RUN cmake --build . --target codegen_shared_obj
