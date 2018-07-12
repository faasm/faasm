FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y build-essential \
    sudo \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    cmake \
    ninja-build \
    wget \
    git \
    software-properties-common

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
RUN sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
RUN apt-get update
RUN apt-get install -y clang-6.0

# RUN apt-get remove --purge cmake

RUN apt-get clean autoclean
RUN apt-get autoremove -y

RUN ln -s /usr/bin/clang-6.0 /usr/bin/clang
RUN ln -s /usr/bin/clang++-6.0 /usr/bin/clang++
RUN ln -s /usr/bin/llvm-6.0 /usr/bin/llvm

# Set up cmake
# WORKDIR /tmp/cmake
# RUN wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
# RUN tar -xvf cmake-3.10.3.tar.gz

# WORKDIR cmake-3.10.3
# RUN ./bootstrap
# RUN make
# RUN make install

# Install WAVM
WORKDIR /faasm/lib
RUN git clone https://github.com/AndrewScheidecker/WAVM
WORKDIR WAVM
RUN mkdir build
WORKDIR build
RUN cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN cmake --build . --target all

# Build code
RUN export CXX=/usr/bin/clang++
RUN export CC=/usr/bin/clang
RUN export CPP=/usr/bin/clang
RUN export LINK=/usr/bin/clang++
RUN export CXX_host=/usr/bin/clang++
RUN export CC_host=/usr/bin/clang
RUN export CPP_host=/usr/bin/clang
RUN export LINK_host=/usr/bin/clang++

# Install cpp_redis (see https://github.com/Cylix/cpp_redis/wiki/Mac-&-Linux-Install)
WORKDIR /faasm/lib
RUN git clone https://github.com/Cylix/cpp_redis.git
WORKDIR cpp_redis
RUN git submodule init && git submodule update
WORKDIR build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN make
RUN make install

WORKDIR /faasm/src
COPY . .

WORKDIR build
RUN cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN make

CMD ./bin/worker
