from performance.benchmarks.bm_unpack_sequence import bench_tuple_unpacking, bench_list_unpacking


def bench_all(loops):
    dt1 = bench_tuple_unpacking(loops)
    dt2 = bench_list_unpacking(loops)
    return dt1 + dt2


if __name__ == "__main__":
    benchmarks = {"tuple": bench_tuple_unpacking,
                  "list": bench_list_unpacking}

    loops = 5
    bench_all(loops)
