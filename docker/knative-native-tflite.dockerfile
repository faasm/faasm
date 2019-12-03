FROM faasm/knative-native-base

ENV FAASM_FUNC=tf
ENV FAASM_USER=image
ENV LD_LIBRARY_PATH=/usr/local/lib

COPY . /usr/local/code/faasm

# Tidy up a bit
WORKDIR /usr/local/code/faasm
RUN rm -r third-party/WAVM third-party/eigen third-party/malloc

