FROM faasm/cpp-root:0.4.11
ARG FAASM_VERSION

WORKDIR /code
RUN git clone -b v${FAASM_VERSION} https://github.com/faasm/faasm
                                                                                
# Install Faasm native libraries
WORKDIR /build/faasm
RUN cmake \
    -GNinja \
    -DFAASM_BUILD_TYPE=native-tools \
    -DFAASM_STATIC_LIBS=OFF \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    -DCMAKE_BUILD_TYPE=Release \
    /code/faasm

RUN ninja
RUN ninja install

ENV FAASM_THIRD_PARTY_LIBS=/build/faasm/third-party/lib

WORKDIR /code/faasm
