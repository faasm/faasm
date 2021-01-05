import sys
import os
from decimal import Decimal
import weakref


class SomeClass:
    pass


def faasm_main():
    print("Running Python version {}".format(sys.version))

    py_wasm_env = os.getenv("PYTHONWASM", "NOT SET")
    py_utf8 = os.getenv("PYTHONUTF8", "NOT SET")
    print("PYTHONWASM={}".format(py_wasm_env))
    print("PYTHONUTF8={}".format(py_utf8))

    x = "foo"
    assert x == "foo"
    print("Assertion passed")

    print("stdout works", file=sys.stdout)
    print("stderr works", file=sys.stderr)

    a1 = 2
    a2 = 3
    print("2 + 3 = ", a1 + a2)

    list_a = list(range(10))
    print("List get 1={} 2={} 3={}".format(list_a[1], list_a[2], list_a[3]))

    list_a[1] = 9
    list_a[3] = 33
    list_a[-3:] = [40, 41, 42]

    print("List: {}".format(list_a))

    d = {"foo": "bar", "baz": 23}

    d["blah"] = Decimal(10)
    d["wah"] = 1.2345

    print("Dict: {}".format(d))

    for k, v in d.items():
        print("{}: {} ({})".format(k, v, d[k]))

    # Weakref check
    c = SomeClass()
    r = weakref.ref(c)
    c2 = r()
    assert c is c2, "Weakref references not equal"
    print("Weakref check successful")

    return 0
