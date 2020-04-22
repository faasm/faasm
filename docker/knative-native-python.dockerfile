ARG FAASM_VERSION
FROM faasm/knative-native-base:$FAASM_VERSION

ENV FAASM_FUNC=py_func
ENV FAASM_USER=python
ENV LD_LIBRARY_PATH=/usr/local/lib

WORKDIR /usr/local/code/faasm

# Install python deps
RUN apt-get install -y libpython3-dev \
    python3-dev \
    python3-pip

RUN pip3 install -U pip

# Copy script to build unoptimized numpy
COPY bin/build_unoptimized_numpy.sh bin/build_unoptimized_numpy.sh

# Install unoptimized numpy
RUN bin/build_unoptimized_numpy.sh

# Install normal numpy
# RUN pip3 install numpy

RUN pip3 install flask \
    gunicorn \
    pyfaasm \
    redis

WORKDIR /usr/local/code/faasm/func
COPY func/knative_native.py .
COPY func/run_knative_native.sh .

CMD ./run_knative_native.sh