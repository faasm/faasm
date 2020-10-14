ARG FAASM_VERSION

# Stage for getting toolchain etc.
FROM faasm/cpython:0.0.2

# Stage for the actual build
FROM faasm/worker:$FAASM_VERSION

# Copy Faasm local dir
COPY --from=0 /usr/local/faasm /usr/local/faasm

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

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc
CMD /bin/bash

