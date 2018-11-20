FROM faasm/base

# Build the upload binary
COPY . /faasm/code
WORKDIR /faasm/build
RUN cmake --build . --target upload

CMD "/faasm/build/bin/upload"