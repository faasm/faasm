FROM faasm/base

COPY . /usr/local/code/faasm

RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook eigen.yml

WORKDIR /faasm/build
RUN cmake --build . --target reuters -- -j

CMD "/bin/bash"