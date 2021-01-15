FILENAME = "lib/python3.8/ctypes/__init__.py"


def faasm_main():
    with open(FILENAME, "rb") as fh:
        contents = fh.read()

    decoded = contents.decode(encoding="utf-8", errors="strict")

    print(decoded)

    return 0
