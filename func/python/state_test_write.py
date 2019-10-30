from pyfaasm.core import getState, getStateOffset, setState, setStateOffset, pushState, pullState

key = "pyStateTest"
valueLen = 10
fullValue = b'0123456789'

segment = b'999'
offset = 2
segmentLen = 3
modifiedValue = b'0199956789'


def _check_full_value(expected):
    pullState(key, valueLen)
    actual = getState(key, valueLen)
    if actual != expected:
        msg = "Mismatch: actual {}, expected {})".format(actual, expected)
        print(msg)
        exit(1)


# Set the full value initially
setState(key, fullValue)
pushState(key)

# Check the full value has been written
_check_full_value(fullValue)

# Update a segment
setStateOffset(key, valueLen, offset, segment)
pushState(key)

# Check the full value again
_check_full_value(modifiedValue)

# Check just the segment
actualSegment = getStateOffset(key, valueLen, offset, segmentLen)
if actualSegment != segment:
    msg = "Mismatched segment: actual {}, expected {})".format(actualSegment, segment)
    print(msg)
    raise RuntimeError(msg)

print("Successful Python state writing check")
