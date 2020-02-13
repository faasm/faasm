ARG FAASM_VERSION
FROM faasm/worker:$FAASM_VERSION

SHELL ["/bin/bash", "-c"]

# Install various deps
RUN apt-get install -y libpython3-dev \
    python3-dev \
    python3-pip \
    python3-venv \
    libcairo2-dev \
    python3-cairo \
    vim

# Python requirements
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Copy code into place
COPY . /usr/local/code/faasm

# Install Faasm native tools
WORKDIR /faasm/native_tools
COPY . /usr/local/code/faasm
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=native-tools \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAASM_STATIC_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    /usr/local/code/faasm

RUN ninja
RUN ninja install

# Download the toolchain
WORKDIR /usr/local/code/faasm
RUN inv download-toolchain
RUN inv download-sysroot

# Install pyfaasm
RUN pip3 install pyfaasm

# Remove worker entrypoint
COPY bin/noop-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc
CMD /bin/bash

