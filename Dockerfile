FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y software-properties-common wget

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
RUN apt-add-repository ppa:ansible/ansible

RUN apt-get update
RUN apt-get install -y build-essential \
    sudo \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    cmake \
    ninja-build \
    git \
    clang-6.0 \
    ansible

RUN apt-get clean autoclean
RUN apt-get autoremove -y

RUN ln -s /usr/bin/clang-6.0 /usr/bin/clang
RUN ln -s /usr/bin/clang++-6.0 /usr/bin/clang++
RUN ln -s /usr/bin/llvm-6.0 /usr/bin/llvm

# Run Ansible set-up
WORKDIR /faasm/code
COPY ansible /faasm/code/ansible
WORKDIR ansible
RUN ansible-playbook libs.yml
RUN ansible-playbook dev.yml

# Build code
RUN export CXX=/usr/bin/clang++
RUN export CC=/usr/bin/clang
RUN export CPP=/usr/bin/clang
RUN export LINK=/usr/bin/clang++
RUN export CXX_host=/usr/bin/clang++
RUN export CC_host=/usr/bin/clang
RUN export CPP_host=/usr/bin/clang
RUN export LINK_host=/usr/bin/clang++

# Work on source
WORKDIR /faasm/code
