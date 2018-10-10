FROM shillaker/cpp-base

RUN apt-add-repository ppa:ansible/ansible
RUN apt-get update
RUN apt-get install -y ansible \
    cgroup-bin \
    iproute2 \
    iptables

COPY . /faasm/code

# Install required libraries
WORKDIR /faasm/code/ansible
RUN ansible-playbook libs.yml

# Build WAVM to avoid repetition in other dockerfiles
WORKDIR /faasm/code/build
RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --target wavm-run


