FROM faasm/base

COPY . /usr/local/code/faasm

RUN apt-get install -y python3-dev python3-pip
RUN pip3 install invoke requests

WORKDIR /faasm/build
RUN cmake --build . --target reuters -- -j

CMD "/bin/bash"