ARG FAASM_VERSION
FROM faasm/base:$FAASM_VERSION

# Nuke existing build
WORKDIR /tmp
RUN rm -rf /faasm/build

# Put code in place
COPY . /usr/local/code/faasm

# Build the knative native runner
# NOTE - need to switch off static libs to support proper dynamic linking of
# Faasm native functions at runtime
WORKDIR /faasm/build
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=knative-native \
    -DFAASM_STATIC_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

# Build the required endpoints
RUN cmake --build . --target faasm
RUN cmake --build . --target emulator
RUN cmake --build . --target knative_native

CMD "/bin/bash"