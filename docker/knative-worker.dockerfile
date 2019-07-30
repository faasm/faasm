FROM faasm/worker

COPY . /usr/local/code/faasm

# Install pistache for HTTP
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook pistache.yml

# Build the knative worker binary
WORKDIR /faasm/build
RUN cmake --build . --target worker-knative -- -j

CMD "/faasm/build/bin/worker-knative"