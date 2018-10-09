FROM shillaker/faasm-base

# Build code
COPY . /faasm/code
WORKDIR /faasm/code/build
RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --target all
