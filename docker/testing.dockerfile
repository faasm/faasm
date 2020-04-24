ARG FAASM_VERSION
FROM faasm/base-test:${FAASM_VERSION}

# -------------------------------------------------------
# This image should contain everything that needs to be 
# rebuilt on every test run, i.e. everything that depends
# on the Faasm source (rather than 3rd party code).
#
# It must also generate all necessary machine code as part 
# of this build to ensure it's correct for the machine on
# which it's built (which will also run it).
# -------------------------------------------------------

COPY . /usr/local/code/faasm

# Build native libraries
WORKDIR /usr/local/code/faasm
RUN inv -r faasmcli/faasmcli libs.native

# Build the actual tests (do this early to catch failures)
WORKDIR /faasm/build
RUN cmake --build . --target tests

# Download tools
WORKDIR /usr/local/code/faasm
RUN inv -r faasmcli/faasmcli toolchain.download-sysroot
RUN inv -r faasmcli/faasmcli toolchain.download-runtime --nocodegen
RUN inv -r faasmcli/faasmcli codegen.local

# Compile test library
RUN inv -r faasmcli/faasmcli libs.fake

# Fix ownership
RUN chown -R root:root /usr/local/faasm

# Install pyfaasm
RUN pip3 install pyfaasm

# Set up Python files
RUN inv -r faasmcli/faasmcli upload.user python --py --local-copy

# Create user with dummy uid required by Python
RUN groupadd -g 1000 faasm
RUN useradd -u 1000 -g 1000 faasm

# Run the tests (when container is run)
WORKDIR /usr/local/code/faasm
CMD /faasm/build/bin/tests
