FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y software-properties-common sudo wget

# Install LLVM and clang
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main"
RUN apt-get update
RUN apt-get install -y build-essential \
    llvm-8 \
    llvm-8-dev \
    llvm-8-tools \
    clang-8

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
RUN ln -s /usr/bin/clang-8 /usr/bin/clang
RUN ln -s /usr/bin/clang-cpp-8 /usr/bin/clang-cpp
RUN ln -s /usr/bin/clang++-8 /usr/bin/clang++
RUN ln -s /usr/bin/llvm-8 /usr/bin/llvm

# Protobuf - note the "no-same-owner" here to avoid inheriting dodgy perms from archive
WORKDIR /tmp
RUN curl -O -L https://github.com/google/protobuf/releases/download/v3.6.0/protobuf-cpp-3.6.0.tar.gz
RUN tar --no-same-owner -xf protobuf-cpp-3.6.0.tar.gz

WORKDIR /tmp/protobuf-3.6.0
RUN ./configure --prefix=/usr CC=/usr/bin/clang CPP=/usr/bin/clang-cpp CXX=/usr/bin/clang++
RUN make
RUN make install
RUN ldconfig

# Remove source
WORKDIR /
RUN rm -rf /tmp/protobuf-3.6.0

CMD /bin/bash
