import pyperformance.benchmarks.bm_json_dumps as bm
from pyperformance.benchmarks.bm_json_dumps import CASES, bench_json_dumps

if __name__ == "__main__":
    cases = CASES

    for x in range(20):
        data = []

        for case in cases:
            obj, count = bm.__dict__[case]
            data.append((obj, range(count)))

        bench_json_dumps(data)
