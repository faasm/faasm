FROM faasm/worker

COPY . /usr/local/code/faasm

WORKDIR /faasm/build
RUN cmake --build . --target worker_knative -- -j

CMD "/faasm/build/bin/worker_knative"