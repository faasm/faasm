# Stage to extract Python runtime files
FROM faasm.azurecr.io/cpython:0.2.4 as python

# Note - we don't often rebuild cpp-root so this dep may be behind
FROM faasm.azurecr.io/cpp-root:0.9.5
ARG FAASM_VERSION

# Flag to say we're in a container
ENV FAASM_DOCKER="on"

# Copy Python runtime libraries
COPY --from=python /usr/local/faasm/runtime_root /usr/local/faasm/runtime_root

SHELL ["/bin/bash", "-c"]

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
RUN cd /usr/local/code/faasm \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && inv -r faasmcli/faasmcli dev.tools \
        --clean \
        --build Release \
        --sgx Disabled
