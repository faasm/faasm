FROM alpine:3.10

# -----------------------------------
# Minimal Dockerfile for runtime benchmarking
# -----------------------------------

RUN apk add gcc musl-dev g++

WORKDIR /faasm/

COPY func/demo/noop.cpp .
RUN g++ noop.cpp -o noop
