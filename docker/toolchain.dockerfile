FROM ubuntu:18.04

SHELL ["/bin/bash", "-c"]

# System deps
RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    build-essential \
    cmake \
    libtool \
    make \
    python3-dev \
    python3-pip \
    python3-venv \
    software-properties-common \
    vim \
    wget \
    python-dev \
    curl

RUN apt-get clean autoclean
RUN apt-get autoremove -y

# Set up Python
RUN pip3 install -U pip
RUN pip3 install -U cryptography
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Copy code into place
COPY . /usr/local/code/faasm
WORKDIR /usr/local/code/faasm

# Download the toolchain
RUN inv download-toolchain
RUN inv download-runtime-root

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc
CMD /bin/bash
