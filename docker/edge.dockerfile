FROM faasm/base

# Install Pistache
WORKDIR /tmp/
RUN git clone https://github.com/oktal/pistache.git
WORKDIR /tmp/pistache

# Checkout a fixed commit that we know works
RUN git checkout fc16ec32e8364e5e47fb367e9fe0da05e275f5cf

RUN git submodule update --init
WORKDIR /tmp/pistache/build

RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
RUN make
RUN sudo make install

# Build the edge binary
COPY . /usr/local/code/faasm
WORKDIR /faasm/build
RUN cmake --build . --target edge

CMD "/usr/local/code/faasm/build/bin/edge"