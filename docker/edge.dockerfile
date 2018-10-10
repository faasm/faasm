FROM shillaker/faasm-base

# Build the edge binary
COPY . /faasm/code
WORKDIR /faasm/build
RUN cmake --build . --target edge

CMD "/faasm/build/bin/edge"