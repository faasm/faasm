FROM faasm/base

# Build the upload and codegen targets
COPY . /usr/local/code/faasm
WORKDIR /faasm/build

RUN cmake --build . --target upload -- -j
RUN cmake --build . --target codegen -- -j

# Set up entrypoint
COPY bin/upload-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

CMD "/faasm/build/bin/upload"