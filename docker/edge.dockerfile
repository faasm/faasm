FROM faasm/base

# Install Pistache
WORKDIR /tmp/
RUN git clone https://github.com/oktal/pistache.git
WORKDIR /tmp/pistache

# Checkout a fixed commit that we know works
# NOTE - make sure this matches the one in the ansible playbooks
RUN git checkout ccf0ab860760e84832e4d484336fee477458c7d2

RUN git submodule update --init
WORKDIR /tmp/pistache/build

RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
RUN make
RUN sudo make install
RUN ldconfig

# Build the edge binary
COPY . /usr/local/code/faasm
WORKDIR /faasm/build
RUN cmake --build . --target edge

CMD "/faasm/build/bin/edge"