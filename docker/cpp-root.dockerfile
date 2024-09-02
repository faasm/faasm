FROM faasm.azurecr.io/faabric-base:0.21.0

# Install Faasm-specific APT dependencies
RUN apt update \
    && apt upgrade -y \
    && apt install -y \
        cgroup-tools \
        iproute2 \
        iptables \
        libcgroup-dev \
        libnsl-dev \
        software-properties-common \
    && apt clean autoclean -y \
    && apt autoremove -y
