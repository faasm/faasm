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

# Put code in place
COPY . /usr/local/code/faasm

# Install native dependencies
WORKDIR /usr/local/code/faasm/ansible

RUN ansible-playbook spdlog.yml
RUN ansible-playbook cereal.yml

RUN ansible-playbook rapidjson.yml
RUN ansible-playbook eigen.yml

RUN ansible-playbook pistache.yml

# Out of tree build
WORKDIR /faasm/build

# Build WAVM to avoid repetition in other dockerfiles
RUN cmake \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target libWAVM -- -j
