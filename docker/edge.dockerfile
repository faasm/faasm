FROM faasm/base

COPY . /usr/local/code/faasm

# Build the edge binary
WORKDIR /faasm/build
RUN cmake --build . --target edge -- -j

CMD "/faasm/build/bin/edge"
