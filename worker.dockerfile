FROM shillaker/faasm-base

# Set up wasm execution stuff
WORKDIR /faasm/code/ansible
RUN ansible-playbook wavm.yml

COPY . /faasm/code
WORKDIR /faasm/code/build

RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Debug ..
RUN cmake --build . --target all -- -j 2

CMD ./bin/worker