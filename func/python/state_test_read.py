from pyfaasm.core import get_state, get_state_offset


def _check_segment(key, offset, length, expected):
    actual = get_state_offset(key, length, offset, length)
    if actual != expected:
        msg = "Mismatched segment: actual {}, expected {})".format(actual, expected)
        print(msg)
        exit(1)


def faasm_main():
    key = "pyStateTest"
    value_len = 10
    expected = b'0199956789'

    # Check the full value
    actual = get_state(key, value_len)
    if actual != expected:
        msg = "Mismatch: actual {}, expected {}".format(actual, expected)
        print(msg)
        exit(1)

    # Check a couple of segments
    _check_segment(key, 0, 3, b'019')
    _check_segment(key, 4, 4, b'9567')

    print("Successful Python state reading check")
