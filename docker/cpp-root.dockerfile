FROM faasm/faabric-base:0.2.2

# -------------------------------------------------------------
# NOTE - extensive use of Ansible in this Dockerfile makes it
# poor at taking advantage of the Docker build cache, so it can
# be a pain to develop. This is a necessary evil that avoids the
# need to create and maintain install/ setup code in this Dockerfile
# that already exists in the Ansible playbooks.
# -------------------------------------------------------------

RUN apt-get update
RUN apt-get install -y software-properties-common
# Make sure packages from faabric-base are up-to-date
RUN apt-get upgrade -y

# Faasm-specific dependencies
RUN apt-get install -y \
    ansible \
    cgroup-tools \
    iproute2 \
    iptables \
    libcgroup-dev

RUN apt-get clean autoclean
RUN apt-get autoremove

CMD /bin/bash
