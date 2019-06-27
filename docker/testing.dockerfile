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

# Set up requests
RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

# Set up eigen
COPY ansible /usr/local/code/faasm/ansible
WORKDIR /usr/local/code/faasm/ansible
RUN ansible-playbook eigen.yml

# Set up pistache
RUN ansible-playbook pistache.yml

# Set up catch
RUN ansible-playbook catch.yml

# Fix ownership of runtime root
RUN chown -R root:root /usr/local/faasm

# Remove any existing code (will check out and rebuild as part of circle job)
WORKDIR /
RUN rm -rf /usr/local/code/faasm
