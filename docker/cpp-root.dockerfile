FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y software-properties-common sudo wget

# Install LLVM and clang
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-6.0 main"
RUN apt-get update
RUN apt-get install -y build-essential \
    llvm-6.0 \
    llvm-6.0-dev \
    llvm-6.0-tools \
    clang-6.0

# Install other deps
RUN apt-get install -y sudo \
    libboost-all-dev \
    make \
    cmake \
    ninja-build \
    git \
    curl \
    autoconf \
    automake \
    libtool \
    unzip

RUN apt-get clean autoclean
RUN apt-get autoremove
# Symlinks
RUN ln -s /usr/bin/clang-6.0 /usr/bin/clang
RUN ln -s /usr/bin/clang-cpp-6.0 /usr/bin/clang-cpp
RUN ln -s /usr/bin/clang++-6.0 /usr/bin/clang++
RUN ln -s /usr/bin/llvm-6.0 /usr/bin/llvm

# Protobuf
WORKDIR /tmp
RUN curl -O -L https://github.com/google/protobuf/releases/download/v3.6.0/protobuf-cpp-3.6.0.tar.gz
RUN tar xvf protobuf-cpp-3.6.0.tar.gz

WORKDIR /tmp/protobuf-3.6.0
RUN ./configure --prefix=/usr CC=/usr/bin/clang CPP=/usr/bin/clang-cpp CXX=/usr/bin/clang++
RUN make
RUN make install
RUN ldconfig

CMD /bin/bash
