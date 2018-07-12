FROM shillaker/faasm-base

COPY . /faasm/code
WORKDIR /faasm/code/build

RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --target all

CMD ./build/bin/edge