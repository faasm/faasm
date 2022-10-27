# Stage to extract Python runtime files
FROM faasm/cpython:0.1.1 as python

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm/cpp-root:0.8.15
ARG FAASM_VERSION

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Python runtime libraries
COPY --from=python /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root

# Check out code (clean beforehand just in case). We also add the code
# directory as a safe Git path. See:
# https://github.blog/2022-04-12-git-security-vulnerability-announced/
RUN rm -rf /usr/local/code/faasm \
    && git clone \
        -b v${FAASM_VERSION} \
        https://github.com/faasm/faasm \
        /usr/local/code/faasm \
    && cd /usr/local/code/faasm \
    && git submodule update --init \
    && git config --global --add safe.directory /usr/local/code/faasm

# Set up runtime filesystem
RUN mkdir -p /usr/local/faasm/runtime_root/etc \
    && cp /usr/local/code/faasm/deploy/conf/hosts /usr/local/faasm/runtime_root/etc/ \
    && cp /usr/local/code/faasm/deploy/conf/resolv.conf /usr/local/faasm/runtime_root/etc/ \
    && cp /usr/local/code/faasm/deploy/conf/passwd /usr/local/faasm/runtime_root/etc/ \
    && mkdir -p /usr/local/faasm/runtime_root/tmp \
    && mkdir -p /usr/local/faasm/runtime_root/share

# Out of tree clean build of the basic targets
RUN rm -rf /build/faasm \
    && mkdir -p /build/faasm \
    && cd /build/faasm \
    && cmake \
        -GNinja \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++-13 \
        -DCMAKE_C_COMPILER=/usr/bin/clang-13 \
        -DCMAKE_BUILD_TYPE=Release \
        -DFAASM_SGX_MODE=Disabled \
        /usr/local/code/faasm \
    && cmake --build . --target tests \
    && cmake --build . --target func_runner \
    && cmake --build . --target codegen_func \
    && cmake --build . --target codegen_shared_obj
