FROM faasm/worker

# Make sure CI-related requirements exist
RUN apt-get update
RUN apt-get install -y apt-transport-https
RUN apt-get install -y \
    git \
    ssh \
    tar \
    gzip \
    ca-certificates \
    libcairo2-dev \
    python3-dev \
    python3-venv

# Set up requests
RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

# Install other python deps
WORKDIR /usr/local/code/faasm
RUN pip3 install -r requirements.txt

# Set up eigen
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook eigen.yml

# Set up pistache
RUN ansible-playbook pistache.yml

# Set up catch
RUN ansible-playbook catch.yml

# Download toolchain
WORKDIR /usr/local/code/faasm
RUN inv download-toolchain

# Fix ownership of runtime root
RUN chown -R root:root /usr/local/faasm

# Build the tests and codegen
WORKDIR /faasm/build
RUN cmake --build . --target tests -- -j
RUN cmake --build . --target codegen_shared_obj -- -j
RUN cmake --build . --target codegen_func -- -j

# Command to run the tests
CMD /faasm/build/bin/tests