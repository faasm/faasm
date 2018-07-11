FROM ubuntu:16.04

RUN apt-get update
RUN apt-get install -y build-essential \
    sudo \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    clang-3.6 \
    clang-format-3.6 \
    ninja-build \
    wget \
    git \
    software-properties-common

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
RUN sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
RUN apt-get update
RUN apt-get install -y clang-6.0

# Set up cmake
WORKDIR /tmp/cmake
RUN apt-get remove --purge cmake
RUN wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
RUN tar -xvf cmake-3.10.3.tar.gz

WORKDIR /tmp/cmake/cmake-3.10.3
RUN ./bootstrap
RUN make
RUN make install

# Install cpp_redis (see https://github.com/Cylix/cpp_redis/wiki/Mac-&-Linux-Install)
WORKDIR /faasm/lib
RUN git clone https://github.com/Cylix/cpp_redis.git
WORKDIR /faasm/lib/cpp_redis
RUN git submodule init && git submodule update
WORKDIR /faasm/lib/cpp_redis/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN make
RUN make install

# Build code
ENV CXX=/usr/bin/clang++
ENV CC=/usr/bin/clang
ENV CPP=/usr/bin/clang
ENV LINK=/usr/bin/clang++
ENV CXX_host=/usr/bin/clang++
ENV CC_host=/usr/bin/clang
ENV CPP_host=/usr/bin/clang
ENV LINK_host=/usr/bin/clang++

WORKDIR /faasm/src
COPY . .

WORKDIR /faasm/src/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make

CMD ./bin/worker