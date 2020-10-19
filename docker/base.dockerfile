# Stages to extract toolchain and sysroot
FROM faasm/sysroot:0.0.10
FROM faasm/cpython:0.0.6 

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm/cpp-root:0.4.11

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Faasm local dir
COPY --from=0 /usr/local/faasm /usr/local/faasm
COPY --from=1 /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root

# Put code in place
COPY . /usr/local/code/faasm

# Install Eigen
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook eigen.yml

# Out of tree build
WORKDIR /faasm/build

# Build WAVM and WAMR to avoid repetition in other dockerfiles
RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target libWAVM
RUN cmake --build . --target wamrlib
