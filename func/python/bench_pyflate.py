import os

from pyperformance.benchmarks.bm_pyflate import bench_pyflake

if __name__ == "__main__":
    file_path = "/lib/python3.7/site-packages/performance/benchmarks/data/interpreter.tar.bz2"

    if not os.environ.get("PYTHONWASM") == "1":
        # Native
        file_path = "/usr/local/faasm/runtime_root" + file_path

    bench_pyflake(1, file_path)
