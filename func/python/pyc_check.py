import bisect
import dis
import marshal
import io


def faasm_main():
    # Python 3.6+ has a 16-byte header
    header_size = 16

    # Get path to pyc file
    pycfile = getattr(bisect, "__cached__")

    # Load the pyc file
    with open(pycfile, "rb") as f:
        # Skip the header
        f.read(header_size)

        # Read the rest of the code
        code = marshal.load(f)

    # Run the disassembly
    disassembly = io.StringIO()
    dis.dis(x=code, file=disassembly)

    disassembly.seek(0)
    disassembly_str = disassembly.read()
    lines = disassembly_str.split("\n")

    actual = lines[:3]

    expected = [
        "  1           0 LOAD_CONST               0 ('Bisection algorithms.')",
        "              2 STORE_NAME               0 (__doc__)",
        "",
    ]

    if actual != expected:
        actual = "\n".join(actual)
        print("Disassembly not as expected. Actual:\n{}".format(actual))
        return 1
    else:
        print("Disassembly as expected")
        return 0
