import json

from pyperformance.benchmarks.bm_json_loads import (
    DICT,
    TUPLE,
    DICT_GROUP,
    bench_json_loads,
)


def faasm_main():
    json_dict = json.dumps(DICT)
    json_tuple = json.dumps(TUPLE)
    json_dict_group = json.dumps(DICT_GROUP)
    objs = (json_dict, json_tuple, json_dict_group)

    for x in range(100):
        bench_json_loads(objs)


if __name__ == "__main__":
    faasm_main()
