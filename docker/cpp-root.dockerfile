FROM faasm/faabric-base:0.3.0

RUN apt-get update
RUN apt-get install -y software-properties-common

# Make sure packages from faabric-base are up-to-date
RUN apt-get upgrade -y

# LLVM setup
RUN add-apt-repository -y -n "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-12 main"

RUN apt-get install -y \
    llvm-12-dev

# Faasm-specific dependencies
RUN apt-get install -y \
    cgroup-tools \
    iproute2 \
    iptables \
    libcgroup-dev

RUN apt-get clean autoclean
RUN apt-get autoremove

CMD /bin/bash
