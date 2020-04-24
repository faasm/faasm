FROM alpine

RUN apk update && \
    apk add g++ libgomp make

# Compile omp programs
COPY func/omp /build
WORKDIR /build
RUN make

# clean up
RUN rm -rf /build
WORKDIR /
