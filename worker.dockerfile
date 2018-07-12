FROM shillaker/faasm-base

# Set up wasm execution stuff
WORKDIR /faasm/code/ansible
RUN ansible-playbook wasm.yml

COPY . /faasm/code
WORKDIR /faasm/code/build

RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --target all

CMD ./build/bin/worker