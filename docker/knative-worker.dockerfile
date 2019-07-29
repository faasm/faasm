FROM faasm/http

# Build the knative worker binary
COPY . /usr/local/code/faasm
WORKDIR /faasm/build
RUN cmake --build . --target worker-knative -- -j

CMD "/faasm/build/bin/worker-knative"