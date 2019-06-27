FROM faasm/worker

# Make sure circle requirements exist
RUN apt-get update
RUN apt-get install -y apt-transport-https
RUN apt-get install -y \
    git \
    ssh \
    tar \
    gzip \
    ca-certificates

# TODO - rebuild upstream cpp-root container to remove protobuf source there
# Has bad permissions that mess up circleci
RUN rm -rf /tmp/protobuf-3.6.0

# Fix ownership of runtime root
RUN chown -R root:root /usr/local/faasm

# Remove any existing code (will check out and rebuild as part of circle job)
RUN rm -rf /usr/local/code/faasm

