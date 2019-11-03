FROM faasm/base

# Install python lib for python functions
RUN apt-get install -y libpython3-dev \
    python3-dev \
    python3-pip

RUN pip3 install -U pip

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

# Install pyfaasm
RUN pip3 install pyfaasm

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
    -DFAASM_AWS_SUPPORT=OFF \
    /usr/local/code/faasm

# Build the library
RUN cmake --build . --target knative_native -- -j

# Build the JSON call tester
RUN cmake --build . --target post_json_call -- -j

# Build the native runner for py funcs
RUN cmake --build . --target knative_native_runner -- -j

CMD "/bin/bash"