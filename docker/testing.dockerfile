ARG FAASM_VERSION
FROM faasm/base-test:${FAASM_VERSION}

COPY . /usr/local/code/faasm

# Build the codegen targets
WORKDIR /faasm/build
RUN cmake --build . --target codegen_shared_obj -- -j
RUN cmake --build . --target codegen_func -- -j

# Set up latest tools
WORKDIR /usr/local/code/faasm
RUN inv download-sysroot
RUN inv download-runtime-root

# Run codegen
RUN inv run-local-codegen

# Compile test library
RUN inv compile-libfake

# Fix ownership
RUN chown -R root:root /usr/local/faasm

# Install emulator etc.
RUN inv install-native-tools

# Install pyfaasm
RUN pip3 install pyfaasm

# Set up Python files
RUN inv upload-all --py --local-copy

# Build the actual tests
RUN cmake --build . --target tests -- -j

# Create user with dummy uid required by Python
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

# Run the tests (when container is run)
CMD /faasm/build/bin/tests
