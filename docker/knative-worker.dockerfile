FROM faasm/worker

COPY . /usr/local/code/faasm

# Install pistache for HTTP
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook pistache.yml

# Build the knative worker binary
WORKDIR /faasm/build
RUN cmake --build . --target worker_knative -- -j

CMD "/faasm/build/bin/worker_knative"