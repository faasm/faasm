FROM faasm/grpc-root:0.0.20

# -------------------------------------------------------------
# NOTE - extensive use of Ansible in this Dockerfile makes it
# poor at taking advantage of the Docker build cache, so it can 
# be a pain to develop. This is a necessary evil that avoids the 
# need to create and maintain install/ setup code in this Dockerfile 
# that already exists in the Ansible playbooks.
# -------------------------------------------------------------

RUN apt-get update

# All apt packages
RUN apt-get install -y \
    ansible \
    autoconf \
    automake \
    cgroup-tools \
    curl \
    iproute2 \
    iptables \
    libboost-dev \
    libboost-all-dev \
    libcpprest-dev \
    libcgroup-dev \
    libcurl4-openssl-dev \
    libhiredis-dev \
    libpython3-dev \
    libtool \
    libz-dev \
    make \ 
    ninja-build \
    redis-tools \
    sudo \
    unzip \
    wget

# Latest cmake
RUN apt remove --purge --auto-remove cmake
WORKDIR /setup
RUN wget -q -O \
    cmake-linux.sh \
    https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-Linux-x86_64.sh
RUN sh cmake-linux.sh -- --skip-license --prefix=/usr/local
WORKDIR /
RUN rm -r /setup

# We could be more tactical here, adding only what's required, thus 
# avoiding invalidating the Docker cache when anything Ansible-related 
# changes. However, this image is not rebuilt often, so it's probably 
# unnecessary.

COPY ansible /usr/local/code/faasm/ansible
WORKDIR /usr/local/code/faasm/ansible

RUN ansible-playbook llvm.yml

RUN apt-get clean autoclean
RUN apt-get autoremove

CMD /bin/bash
