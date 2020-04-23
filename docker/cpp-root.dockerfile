FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y software-properties-common sudo wget

# Note - we have to stick with stable LLVM here for now
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main"
RUN apt-get update
RUN apt-get install -y build-essential \
    llvm-10 \
    llvm-10-dev \
    llvm-10-tools \
    lld-10 \
    clang-10 \
    clang-tools-10 \
    libclang-10-dev \
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
RUN ln -s /usr/bin/clang-10 /usr/bin/clang && \
    ln -s /usr/bin/clang++-10 /usr/bin/clang++ && \
    ln -s /usr/bin/clang-cpp-10 /usr/bin/clang-cpp && \
    ln -s /usr/bin/lld-10 /usr/bin/lld && \
    ln -s /usr/bin/llvm-ar-10 /usr/bin/llvm-ar && \
    ln -s /usr/bin/llvm-config-10 /usr/bin/llvm-config && \
    ln -s /usr/bin/llvm-nm-10 /usr/bin/llvm-nm && \
    ln -s /usr/bin/llvm-ranlib-10 /usr/bin/llvm-ranlib && \
    ln -s /usr/bin/wasm-ld-10 /usr/bin/wasm-ld

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
