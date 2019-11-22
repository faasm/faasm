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

#RUN wget https://files.pythonhosted.org/packages/af/d9/4113a1841a9a0dfdadaee21da5d8b4538d1289cdd8a63bf8edff9f0d7652/pyfaasm-0.1.2.tar.gz
#RUN pip install pyfaasm-0.1.2.tar.gz

RUN pip3 install flask \
    gunicorn \
    numpy \
    pyfaasm \
    redis

WORKDIR /usr/local/code/faasm/func
COPY func/knative_native.py .
COPY func/run_knative_native.sh .

CMD ./run_knative_native.sh