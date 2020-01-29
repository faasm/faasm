ARG FAASM_VERSION
FROM faasm/knative-native-base:$FAASM_VERSION

ARG FAASM_FUNC
ARG FAASM_USER
ENV FAASM_FUNC=$FAASM_FUNC
ENV FAASM_USER=$FAASM_USER

# Copy function code into place
COPY func/${FAASM_USER}/ /usr/local/code/faasm/func/${FAASM_USER}/

# Build tensorflow (will skip if not required)
WORKDIR /usr/local/code/faasm/
RUN ./bin/build_tflite_native.sh

# Build the function
WORKDIR /faasm/build

RUN cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=knative-native \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target ${FAASM_FUNC}-knative

CMD /faasm/build/bin/${FAASM_FUNC}-knative
