FROM faasm/faabric-base:0.3.7

# Install Faasm-specific APT dependencies
RUN apt update \
    && apt upgrade -y \
    && apt install -y \
        cgroup-tools \
        iproute2 \
        iptables \
        libcairo2-dev \
        libcgroup-dev \
        software-properties-common \
    && apt clean autoclean -y \
    && apt autoremove -y
