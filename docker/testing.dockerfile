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

# Remove any existing code (will check out and rebuild as part of circle job)
RUN rm -rf /usr/local/code/faasm
