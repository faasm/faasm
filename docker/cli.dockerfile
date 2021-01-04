ARG FAASM_VERSION
FROM faasm/base:$FAASM_VERSION

SHELL ["/bin/bash", "-c"]

# Install various deps
RUN apt-get update
RUN apt-get install -y \
    clang-tidy-10 \
    libpython3-dev \
    python3-dev \
    python3-pip \
    python3-venv \
    libcairo2-dev \
    python3-cairo \
    vim

# Install wabt
# TODO - pin this to a release
RUN git clone https://github.com/WebAssembly/wabt/ /tmp/wabt
WORKDIR /tmp/wabt/build
RUN cmake -GNinja -DBUILD_TESTS=OFF -DBUILD_LIBWASM=OFF ..
RUN ninja install
WORKDIR /
RUN rm -r /tmp/wabt

# Install faasm toolchain Python module
# TODO - include this with via multi-stage Docker build
WORKDIR /usr/local/code
RUN git clone https://github.com/faasm/faasm-toolchain
WORKDIR /usr/local/code/faasm-toolchain
RUN pip3 install -e .

# Python set-up
WORKDIR /usr/local/code/faasm
RUN pip3 install -r faasmcli/requirements.txt
RUN pip3 install -e faasmcli/

# Build some useful targets
RUN inv -r faasmcli/faasmcli dev.tools --build Release

# Install common libraries
RUN inv -r faasmcli/faasmcli libs.fake

# Remove worker entrypoint
COPY bin/noop-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

ENV TERM xterm-256color

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/bin/workon.sh" >> ~/.bashrc
CMD ["/bin/bash", "-l"]

