ARG FAASM_VERSION
FROM faasm/base:${FAASM_VERSION}

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

# Set up Catch
WORKDIR /usr/local/code/faasm/ansible
COPY ansible/catch.yml catch.yml
RUN ansible-playbook catch.yml

# Set up requests
RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

# Install other python deps
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Download the toolchain
WORKDIR /usr/local/code/faasm
RUN inv download-toolchain

# Clear out
RUN rm -rf /tmp/*
