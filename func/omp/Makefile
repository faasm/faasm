# Makefile for building native_musl targets in the omp-musl container

CXX=g++
CXX_FLAGS=-mavx2 -O3
OMP_FLAGS=-fopenmp
OUT_DIR=/usr/bin

.PHONY : all

all : mt_pi docker_benchmarker

mt_pi: mt_pi.cpp
	$(CXX) $(CXX_FLAGS) $(OMP_FLAGS) $< -o $(OUT_DIR)/$@

docker_benchmarker: docker_benchmarker.cpp
	$(CXX) $(CXX_FLAGS) $< -o $(OUT_DIR)/$@
