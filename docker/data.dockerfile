FROM faasm/base

COPY . /faasm/code

RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

WORKDIR /faasm/code/ansible
RUN ansible-playbook eigen.yml

WORKDIR /faasm/build
RUN cmake --build . --target reuters

CMD "/bin/bash"