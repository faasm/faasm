FROM faasm/worker

# Make sure CI-related requirements exist
RUN apt-get update
RUN apt-get install -y apt-transport-https
RUN apt-get install -y \
    git \
    ssh \
    tar \
    gzip \
    ca-certificates \
    libcairo2-dev \
    python3-dev \
    python3-venv

# Set up requests
RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

# Install other python deps
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt

# Set up Catch
WORKDIR /usr/local/code/faasm/ansible
COPY ansible/catch.yml catch.yml
RUN ansible-playbook catch.yml

# Clear out
RUN rm -rf /tmp/*

# Set up runtime root and sysroot
COPY . /usr/local/code/faasm
WORKDIR /usr/local/code/faasm
RUN inv download-sysroot
RUN chown -R root:root /usr/local/faasm

# Build the local tooling
RUN inv install-native-tools

# Install pyfaasm
RUN pip3 install pyfaasm

# Build the tests
WORKDIR /faasm/build
RUN cmake --build . --target tests -- -j

# Remove worker entrypoint
COPY bin/noop-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

# Command to run the tests
CMD /faasm/build/bin/tests
