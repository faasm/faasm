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
    python-dev

RUN apt-get clean autoclean
RUN apt-get autoremove -y

# Set up Python
RUN pip3 install -U pip
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Download the toolchain
WORKDIR /tmp
RUN wget https://s3-eu-west-1.amazonaws.com/faasm-misc/emsdk.tar.gz -O emsdk.tar.gz
RUN tar -xf emsdk.tar.gz
RUN rm emsdk.tar.gz

# Copy code into place
COPY . /usr/local/code/faasm
WORKDIR /usr/local/code/faasm

# Put toolchain in right place (do this after copying the code to avoid
# downloading the toolchain when the code changes)
WORKDIR /usr/local/code/faasm/pyodide/emsdk
RUN mv /tmp/emsdk emsdk

# Set up context in bashrc
WORKDIR /usr/local/code/faasm
RUN echo ". /usr/local/code/faasm/workon.sh" >> ~/.bashrc

# Build libs
RUN source workon.sh && ./bin/build_musl.sh
RUN source workon.sh && inv compile-eigen
RUN source workon.sh && inv compile-malloc
RUN source workon.sh && inv compile-libfaasm

CMD /bin/bash
