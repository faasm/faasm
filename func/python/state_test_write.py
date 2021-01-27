from pyfaasm.core import (
    read_state,
    read_state_offset,
    write_state_offset,
    push_state,
    pull_state,
    push_state_partial,
    write_state,
)

key = "pyStateTest"
value_len = 10
full_value = b"0123456789"

segment = b"999"
offset = 2
segment_len = 3
modified_value = b"0199956789"


def _check_full_value(expected):
    pull_state(key, value_len)
    actual = read_state(key, value_len)
    if actual != expected:
        msg = "Mismatch: actual {}, expected {})".format(actual, expected)
        print(msg)
        exit(1)


def faasm_main():
    # Set the full value initially
    write_state(key, full_value)
    push_state(key)

    # Check the full value has been written
    _check_full_value(full_value)

    # Update a segment
    write_state_offset(key, value_len, offset, segment)
    push_state_partial(key)

    # Check the full value again
    _check_full_value(modified_value)

    # Check just the segment
    actual_segment = read_state_offset(key, value_len, offset, segment_len)
    if actual_segment != segment:
        msg = "Mismatched segment: actual {}, expected {}".format(
            actual_segment, segment
        )
        print(msg)
        exit(1)

    print("Successful Python state writing check")

    return 0
