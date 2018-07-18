FROM shillaker/faasm-base

COPY . /faasm/code
WORKDIR /faasm/code/build

RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Debug ..
RUN cmake --build . --target all

CMD ./bin/edge_server