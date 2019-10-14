FROM faasm/knative-native-base

ARG FUNC
ARG USER
ENV FUNC=$FUNC
ENV USER=$USER

# Copy function code into place
COPY func/${USER}/ /usr/local/code/faasm/func/${USER}/

# Build the function
WORKDIR /faasm/build

RUN cmake \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DFAASM_BUILD_TYPE=knative-native \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

RUN cmake --build . --target ${FUNC}-knative -- -j

CMD /faasm/build/bin/${FUNC}-knative