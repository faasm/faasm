from pyfaasm.core import getState, getStateOffset, setState, setStateOffset

# Set the full value
key = "pyStateTest"
valueLen = 10
fullValue = b'0123456789'

segment = b'999'
offset = 2
segmentLen = 3
modifiedValue = b'0199956789'

def _check_full_value(expected):
    actual = getState(key, valueLen)
    if actual != expected:
        msg = "Mismatch: actual {}, expected {})".format(actual, expected)
        print(msg)
        raise RuntimeError(msg)

# Set the full value initially
setState(key, fullValue, valueLen)

# Check the full value has been written
_check_full_value(fullValue)

# Update a segment
setStateOffset(key, valueLen, offset, segment, segmentLen)

# Check the full value again
actualValue = getState(key, valueLen)
_check_full_value(modifiedValue)

# Check just the segment
actualSegment = getStateOffset(key, valueLen, offset, segmentLen)
if actualSegment != segment:
    