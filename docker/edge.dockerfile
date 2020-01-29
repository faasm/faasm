FROM faasm/base

COPY . /usr/local/code/faasm

# Build the edge binary
WORKDIR /faasm/build
RUN cmake --build . --target edge

CMD "/faasm/build/bin/edge"
