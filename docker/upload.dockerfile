FROM faasm/base

# Build the upload binary
COPY . /usr/local/code/faasm
WORKDIR /faasm/build
RUN cmake --build . --target upload -- -j

CMD "/faasm/build/bin/upload"