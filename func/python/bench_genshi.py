from pyperformance.benchmarks.bm_genshi import bench_genshi, BENCHMARKS

if __name__ == "__main__":
    for bench in BENCHMARKS:
        tmpl_cls, tmpl_str = BENCHMARKS[bench]
        bench_genshi(3, tmpl_cls, tmpl_str)
