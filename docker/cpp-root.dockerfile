FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y software-properties-common sudo wget

# Note - we have to stick with stable LLVM here for now
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main"
RUN apt-get update
RUN apt-get install -y build-essential \
    llvm-8 \
    llvm-8-dev \
    llvm-8-tools \
    lld-8 \
    clang-8 \
    clang-tools-8 \
    libclang-8-dev \
    make

# Install an up-to-date cmake
RUN apt remove --purge --auto-remove cmake
WORKDIR /usr/local/lib/cmake-3.15.0
RUN wget https://github.com/Kitware/CMake/releases/download/v3.15.0/cmake-3.15.0-Linux-x86_64.sh
RUN chmod +x cmake-3.15.0-Linux-x86_64.sh
RUN ./cmake-3.15.0-Linux-x86_64.sh --skip-license --prefix=/usr/local
RUN rm cmake-3.15.0-Linux-x86_64.sh

# Install other deps
RUN apt-get install -y sudo \
    libboost-all-dev \
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
RUN ln -s /usr/bin/wasm-ld-8 /usr/bin/wasm-ld
RUN ln -s /usr/bin/llvm-ar-8 /usr/bin/llvm-ar
RUN ln -s /usr/bin/lld-8 /usr/bin/lld

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
