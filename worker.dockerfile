FROM shillaker/faasm-base

COPY . /faasm/code

RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --target all

CMD ./build/bin/worker