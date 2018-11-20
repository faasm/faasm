FROM faasm/base

# Install Pistache
WORKDIR /tmp/
RUN git clone https://github.com/oktal/pistache.git
WORKDIR /tmp/pistache
RUN git submodule update --init
WORKDIR /tmp/pistache/build
RUN cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
RUN make
RUN sudo make install

# Build the edge binary
COPY . /faasm/code
WORKDIR /faasm/build
RUN cmake --build . --target edge

CMD "/faasm/build/bin/edge"