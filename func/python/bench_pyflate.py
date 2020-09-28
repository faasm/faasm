import os
from os.path import exists

from pyperformance.benchmarks.bm_pyflate import bench_pyflake


def faasm_main():
    if os.environ.get("PYTHONWASM") == "1":
        file_path = "/lib/python3.8/site-packages/pyperformance/benchmarks/data/interpreter.tar.bz2"
    else:
        file_path = "/usr/local/code/faasm/venv/lib/python3.6/site-packages/pyperformance/benchmarks/data/interpreter.tar.bz2"
        if not exists(file_path):
            file_path = "/usr/local/lib/python3.6/dist-packages/pyperformance/benchmarks/data/interpreter.tar.bz2"

    bench_pyflake(1, file_path)


if __name__ == "__main__":
    faasm_main()
