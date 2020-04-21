from pyperformance.benchmarks.bm_nbody import bench_nbody, DEFAULT_REFERENCE, DEFAULT_ITERATIONS


def faasm_main():
    bench_nbody(10, DEFAULT_REFERENCE, DEFAULT_ITERATIONS)


if __name__ == "__main__":
    faasm_main()
