from pyfaasm.core import getState, getStateOffset, setState, setStateOffset, pushState, pullState, pushStatePartial

key = "pyStateTest"
value_len = 10
full_value = b'0123456789'

segment = b'999'
offset = 2
segment_len = 3
modified_value = b'0199956789'


def _check_full_value(expected):
    pullState(key, value_len)
    actual = getState(key, value_len)
    if actual != expected:
        msg = "Mismatch: actual {}, expected {})".format(actual, expected)
        print(msg)
        exit(1)


def main_func():
    # Set the full value initially
    setState(key, full_value)
    pushState(key)

    # Check the full value has been written
    _check_full_value(full_value)

    # Update a segment
    setStateOffset(key, value_len, offset, segment)
    pushStatePartial(key)

    # Check the full value again
    _check_full_value(modified_value)

    # Check just the segment
    actual_segment = getStateOffset(key, value_len, offset, segment_len)
    if actual_segment != segment:
        msg = "Mismatched segment: actual {}, expected {})".format(actual_segment, segment)
        print(msg)
        raise RuntimeError(msg)

    print("Successful Python state writing check")


if __name__ == "__main__":
    main_func()
