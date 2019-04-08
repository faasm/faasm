FROM faasm/base

# Set up dummy networking files
WORKDIR /faasm/code/ansible
RUN ansible-playbook runtime_fs.yml

# Build the worker binary
COPY . /faasm/code
WORKDIR /faasm/build
RUN cmake --build . --target worker

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/worker-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD "/faasm/build/bin/worker"
