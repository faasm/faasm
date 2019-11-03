FROM faasm/knative-native-base

ENV FAASM_FUNC=py_func
ENV FAASM_USER=python
ENV LD_LIBRARY_PATH=/usr/local/lib

WORKDIR /usr/local/code/faasm

# Install python deps
RUN apt-get install -y libpython3-dev \
    python3-dev \
    python3-pip

RUN pip3 install -U pip

RUN pip3 install flask \
    numpy \
    pyfaasm \
    redis

WORKDIR /usr/local/code/faasm/func
COPY func/knative_native.py .
CMD python3 knative_native.py