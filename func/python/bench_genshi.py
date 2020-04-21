from pyperformance.benchmarks.bm_genshi import bench_genshi, BENCHMARKS


def faasm_main():
    for bench in BENCHMARKS:
        tmpl_cls, tmpl_str = BENCHMARKS[bench]
        bench_genshi(3, tmpl_cls, tmpl_str)


if __name__ == "__main__":
    faasm_main()
