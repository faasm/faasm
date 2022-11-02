ARG FAASM_VERSION
ARG FAASM_SGX_PARENT_SUFFIX
FROM faasm/base${FAASM_SGX_PARENT_SUFFIX}:$FAASM_VERSION

SHELL ["/bin/bash", "-c"]

# Install various deps
RUN apt update \
    && apt install -y \
        clang-tidy-10 \
        clang-tidy-13 \
        doxygen \
        vim

# Install wabt
RUN git clone -b 1.0.29 https://github.com/WebAssembly/wabt/ /tmp/wabt \
    && mkdir -p /tmp/wabt/build \
    && cd /tmp/wabt/build \
    && cmake -GNinja -DBUILD_TESTS=OFF -DBUILD_LIBWASM=OFF .. \
    && ninja install \
    && cd / \
    && rm -r /tmp/wabt

# Python set-up
WORKDIR /usr/local/code/faasm
RUN ./bin/create_venv.sh

# Build some useful targets
ARG FAASM_SGX_MODE
RUN source venv/bin/activate && \
        inv -r faasmcli/faasmcli dev.tools \
        --build Release \
        --sgx ${FAASM_SGX_MODE}

# Remove worker entrypoint
COPY bin/noop-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

 # Terminal colours
ENV TERM xterm-256color

# GDB config, allow loading repo-specific config
RUN touch /root/.gdbinit
RUN echo "set auto-load safe-path /" > /root/.gdbinit

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/bin/workon.sh" >> ~/.bashrc
CMD ["/bin/bash", "-l"]

