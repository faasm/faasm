import pickle


def faasm_main():
    d = {"foo": 123, "bar": "baz"}

    d_pick = pickle.dumps(d)

    d_after = pickle.loads(d_pick)

    if d != d_after:
        print(
            "Dictionary not equal after pickling: {} != {}".format(
                d_pick, d_after
            )
        )
        return 1

    return 0
