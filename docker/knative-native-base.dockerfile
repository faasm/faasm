FROM faasm/base

# Install python lib for python functions
RUN apt-get install libpython3-dev

# Install pyfaasm for interacting with emulator
RUN pip install pyfaasm

COPY . /usr/local/code/faasm

# Nuke and recreate the build dir
WORKDIR /tmp
RUN rm -rf /faasm/build
WORKDIR /faasm/build

# Build the knative native runner
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

CMD "/bin/bash"