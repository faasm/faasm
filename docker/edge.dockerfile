FROM faasm/base

COPY . /usr/local/code/faasm

# Install pistache
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook pistache.yml

# Build the edge binary
WORKDIR /faasm/build
RUN cmake --build . --target edge -- -j

CMD "/faasm/build/bin/edge"