FROM shillaker/cpp-base

RUN apt-add-repository ppa:ansible/ansible
RUN apt-get update
RUN apt-get install -y ansible \
    cgroup-bin \
    iproute2 \
    iptables

COPY . /faasm/code

# Install required libraries
WORKDIR /faasm/code/ansible
RUN ansible-playbook libs.yml

# Set up dummy networking files
RUN ansible-playbook net_files.yml

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

CMD /bin/bash
