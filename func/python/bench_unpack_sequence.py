from pyperformance.benchmarks.bm_unpack_sequence import (
    bench_tuple_unpacking,
    bench_list_unpacking,
)


def bench_all(loops):
    dt1 = bench_tuple_unpacking(loops)
    dt2 = bench_list_unpacking(loops)
    return dt1 + dt2


def faasm_main():
    loops = 5
    bench_all(loops)


if __name__ == "__main__":
    faasm_main()
