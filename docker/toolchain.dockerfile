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
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Python and pyodide deps
RUN apt-get install -y \
    zlib1g-dev \
    gfortran \
    f2c \
    libffi-dev \
    libssl-dev \
    libbz2-dev

# Install local python3.7 (must be exact version match with pyodide's CPython)
WORKDIR /tmp
RUN wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tgz
RUN tar -xf Python-3.7.0.tgz
WORKDIR /tmp/Python-3.7.0
RUN ./configure --prefix=/usr/local/faasm/python3.7
RUN make altinstall

# Copy code into place
# NOTE: do as much heavy lifting as you can *BEFORE* this to make the most of caching when rebuilding
COPY . /usr/local/code/faasm
WORKDIR /usr/local/code/faasm

# Download the toolchain
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook emsdk.yml

# Build CPython and python packages
WORKDIR /usr/local/code/faasm/pyodide
RUN source workon.sh && cd cpython && make clean && make
RUN source workon.sh && cd packages && make clean && make

# Build lib
WORKDIR /usr/local/code/faasm
RUN source workon.sh && ./bin/build_musl.sh
RUN source workon.sh && inv compile-eigen
RUN source workon.sh && inv compile-malloc --clean
RUN source workon.sh && inv compile-libfaasm --clean

# Prepare bashrc
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc
CMD /bin/bash
