FROM faasm/base

# Changing ownership here seems to be necessary to keep circleci happy
COPY . /usr/local/code/faasm
RUN chown -R root:root /usr/local/code/faasm

# Set up dummy networking files
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook runtime_fs.yml

# Build the worker and codegen binaries
WORKDIR /faasm/build
RUN cmake --build . --target worker
RUN cmake --build . --target codegen

# Get the Python runtime root from S3
WORKDIR /usr/local/faasm
RUN wget https://s3-eu-west-1.amazonaws.com/faasm-misc/faasm_runtime_root.tar.gz
RUN tar -xf faasm_runtime_root.tar.gz
RUN rm faasm_runtime_root.tar.gz

# Run codegen on Python stuff
WORKDIR /faasm/build
RUN ./bin/codegen /usr/local/faasm/runtime_root/lib/python3.7
RUN chown -R root:root /usr/local/faasm

# Set up entrypoint (for cgroups, namespaces etc.)
COPY bin/worker-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD "/faasm/build/bin/worker"
