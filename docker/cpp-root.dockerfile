FROM faasm.azurecr.io/faabric-base:0.4.2

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
