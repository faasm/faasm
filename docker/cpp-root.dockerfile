FROM faasm/faabric-base:0.3.5

# Install Faasm-specific APT dependencies
RUN apt update \
    && apt upgrade -y \
    && apt install -y \
        cgroup-tools \
        iproute2 \
        iptables \
        libcgroup-dev \
        software-properties-common \
    && apt clean autoclean -y \
    && apt autoremove -y
