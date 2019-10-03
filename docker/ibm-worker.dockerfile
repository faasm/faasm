FROM faasm/worker

# Although IBM provide a default Docker environment, it uses a Flask HTTP wrapper:
# https://github.com/apache/openwhisk-runtime-docker/tree/master/core/actionProxy
#
# To keep things lightweight we use our existing endpoints.

COPY . /usr/local/code/faasm

# Build the worker
WORKDIR /faasm/build
RUN cmake --build . --target worker_ibm -- -j

CMD "/faasm/build/bin/worker_ibm"