FROM faasm/base

# Install Faasm native tools
WORKDIR /faasm/native_tools
COPY . /usr/local/code/faasm
RUN cmake \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=native-tools \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAASM_STATIC_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    /usr/local/code/faasm

RUN make -j
RUN make install

# Nuke existing build
WORKDIR /tmp
RUN rm -rf /faasm/build

# Build the knative native runner
WORKDIR /faasm/build
RUN cmake \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=knative-native \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

# Build the library
RUN cmake --build . --target knative_native -- -j

CMD "/bin/bash"