FROM shillaker/faasm-base

# Build the edge binary
WORKDIR /faasm/code/build
RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --target edge

CMD "/faasm/code/build/bin/edge"