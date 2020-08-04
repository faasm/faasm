ARG FAASM_VERSION
FROM faasm/worker:$FAASM_VERSION

SHELL ["/bin/bash", "-c"]

# Install various deps
RUN apt-get update
RUN apt-get install -y libpython3-dev \
    python3-dev \
    python3-pip \
    python3-venv \
    libcairo2-dev \
    python3-cairo \
    vim

# Python requirements
COPY faasmcli/requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Copy code into place
COPY . /usr/local/code/faasm

# Build some useful targets
WORKDIR /faasm/build
RUN cmake --build . --target wavm
RUN cmake --build . --target simple_runner
RUN cmake --build . --target codegen_func
RUN cmake --build . --target codegen_shared_obj
RUN cmake --build . --target func_sym

# Install Faasm native tools
WORKDIR /faasm/native_tools
COPY . /usr/local/code/faasm
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    -DFAASM_BUILD_TYPE=native-tools \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAASM_STATIC_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    /usr/local/code/faasm

RUN ninja
RUN ninja install

# Install the CLI
WORKDIR /usr/local/code/faasm
RUN pip3 install -e faasmcli/

# Download the toolchain
RUN inv -r faasmcli/faasmcli toolchain.download-toolchain
RUN inv -r faasmcli/faasmcli toolchain.download-sysroot

# Install pyfaasm
RUN pip3 install pyfaasm

# Remove worker entrypoint
COPY bin/noop-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc
CMD /bin/bash

