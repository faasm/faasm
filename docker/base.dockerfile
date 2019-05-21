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

# Install common libs
COPY . /faasm/code
WORKDIR /faasm/code/ansible
RUN ansible-playbook aws_sdk.yml
RUN ansible-playbook aws_lambda.yml
RUN ansible-playbook rapidjson.yml

# Build WAVM to avoid repetition in other dockerfiles
WORKDIR /faasm/build
RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release /faasm/code
RUN cmake --build . --target wavm-run
