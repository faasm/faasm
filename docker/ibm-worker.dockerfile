FROM faasm/worker

# Main IBM Docker environment is from here:
# https://github.com/apache/openwhisk-runtime-docker/tree/master/core/actionProxy
#
# We provide our own endpoint rather than use theirs

COPY . /usr/local/code/faasm

WORKDIR /faasm/build
RUN cmake --build . --target worker_ibm -- -j

CMD "/faasm/build/bin/worker_ibm"