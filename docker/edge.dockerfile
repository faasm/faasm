FROM shillaker/faasm-base

# Build the edge binary
WORKDIR /faasm/code/build
RUN cmake --build . --target edge

CMD "/faasm/code/build/bin/edge"