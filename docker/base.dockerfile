FROM faasm/cpp-root

RUN apt-get update
RUN apt-add-repository ppa:ansible/ansible
RUN apt-get update
RUN apt-get install -y ansible \
    cgroup-bin \
    iproute2 \
    iptables \
    redis-tools \
    libz-dev \
    libboost-dev \
    libcpprest-dev \
    libhiredis-dev \
    libcgroup-dev \
    libcurl4-openssl-dev \
    cgroup-tools \
    cgroup-bin

WORKDIR /tmp

# Install spdlog
RUN wget https://github.com/gabime/spdlog/archive/v1.2.1.tar.gz
RUN tar -xf v1.2.1.tar.gz
RUN mkdir spdlog-1.2.1/build

WORKDIR /tmp/spdlog-1.2.1/build
RUN cmake ..
RUN make
RUN make install

# AWS SDK
WORKDIR /tmp
RUN git clone https://github.com/aws/aws-sdk-cpp.git
WORKDIR /tmp/aws-sdk-cpp
RUN git checkout 1.7.108
WORKDIR /tmp/aws-sdk-cpp/build
RUN cmake -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_ONLY="lambda;s3;sqs" \
    -DENABLE_TESTING=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DUSTOM_MEMORY_MANAGEMENT=OFF \
    ..
RUN make
RUN make install

# AWS Lambda runtime
WORKDIR /tmp
RUN git clone https://github.com/awslabs/aws-lambda-cpp
WORKDIR /tmp/aws-lambda-cpp
WORKDIR /tmp/aws-lambda-cpp/build
RUN cmake -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    ..
RUN make
RUN make install

# Rapid JSON
WORKDIR /tmp
RUN git https://github.com/Tencent/rapidjson
WORKDIR /tmp/rapidjson
WORKDIR /tmp/rapidjson/build
RUN cmake ..
RUN make
RUN make install

# Put code in place but run out of tree build
COPY . /usr/local/code/faasm
WORKDIR /faasm/build

# Build WAVM to avoid repetition in other dockerfiles
RUN cmake \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target wavm-run
