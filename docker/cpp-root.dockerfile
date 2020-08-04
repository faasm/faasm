FROM ubuntu:18.04

# -------------------------------------------------------------
# NOTE - extensive use of Ansible in this Dockerfile makes it
# poor at taking advantage of the Docker build cache, so it can 
# be a pain to develop. This is a necessary evil that avoids the 
# need to create and maintain install/ setup code in this Dockerfile 
# that already exists in the Ansible playbooks.
# -------------------------------------------------------------

RUN apt-get update
RUN apt-get install -y software-properties-common

# Ansible repo
RUN apt-add-repository -y ppa:ansible/ansible
RUN apt-get update

# All apt packages
RUN apt-get install -y \
    ansible \
    autoconf \
    automake \
    cgroup-bin \
    cgroup-tools \
    curl \
    git \
    iproute2 \
    iptables \
    libboost-dev \
    libboost-all-dev \
    libcpprest-dev \
    libcgroup-dev \
    libcurl4-openssl-dev \
    libhiredis-dev \
    libtool \
    libz-dev \
    make \ 
    ninja-build \
    redis-tools \
    sudo \
    unzip \
    wget 

# Remove any existing cmake 
RUN apt remove --purge --auto-remove cmake

# We could be more tactical here, adding only what's required, thus 
# avoiding invalidating the Docker cache when anything Ansible-related 
# changes. However, this image is not rebuilt often, so it's probably 
# unnecessary.

COPY ansible /usr/local/code/faasm/ansible
WORKDIR /usr/local/code/faasm/ansible

RUN ansible-playbook llvm10.yml

RUN ansible-playbook cmake.yml

RUN ansible-playbook protobuf.yml

RUN ansible-playbook grpc.yml

RUN ansible-playbook spdlog.yml

RUN ansible-playbook cereal.yml

RUN ansible-playbook rapidjson.yml

RUN ansible-playbook pistache.yml

RUN apt-get clean autoclean
RUN apt-get autoremove

CMD /bin/bash
