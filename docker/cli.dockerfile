ARG FAASM_VERSION
FROM faasm/base:$FAASM_VERSION

SHELL ["/bin/bash", "-c"]

# Install various deps
RUN apt-get update
RUN apt-get install -y \
    libpython3-dev \
    python3-dev \
    python3-pip \
    python3-venv \
    libcairo2-dev \
    python3-cairo \
    vim

# Install faasm toolchain module
WORKDIR /usr/local/code
RUN git clone https://github.com/faasm/faasm-toolchain
WORKDIR /usr/local/code/faasm-toolchain
RUN pip3 install -e .

# Build some targets not provided by the worker image
WORKDIR /faasm/build
RUN cmake --build . --target simple_runner
RUN cmake --build . --target func_sym

# Python set-up
WORKDIR /usr/local/code/faasm
RUN pip3 install -r faasmcli/requirements.txt
RUN pip3 install -e faasmcli/

# Set up native tools
RUN inv -r faasmcli/faasmcli libs.native

# Install pyfaasm
RUN pip3 install pyfaasm

# Remove worker entrypoint
COPY bin/noop-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

ENV TERM xterm-256color

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/bin/workon.sh" >> ~/.bashrc
CMD ["/bin/bash", "-l"]

