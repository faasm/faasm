FROM shillaker/cpp-base

RUN apt-add-repository ppa:ansible/ansible
RUN apt-get update
RUN apt-get install -y ansible \
    cgroup-bin \
    iproute2 \
    iptables

COPY ./ansible /faasm/code/ansible

# Install required libraries
WORKDIR /faasm/code/ansible
RUN ansible-playbook libs.yml

