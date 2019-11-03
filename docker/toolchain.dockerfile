FROM faasm/worker

SHELL ["/bin/bash", "-c"]

# Install python deps
RUN apt-get install -y libpython3-dev \
    python3-dev \
    python3-pip

# Python requirements
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Copy code into place
COPY . /usr/local/code/faasm
WORKDIR /usr/local/code/faasm

# Install Faasm native tools
WORKDIR /faasm/native_tools
COPY . /usr/local/code/faasm
RUN cmake \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=native-tools \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAASM_AWS_SUPPORT=OFF \
    -DFAASM_STATIC_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    /usr/local/code/faasm

RUN make -j
RUN make install

# Download the toolchain
RUN inv download-toolchain

# Install pyfaasm
RUN pip3 install pyfaasm

WORKDIR /usr/local/code/faasm

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc
CMD /bin/bash
