from pyfaasm.core import getState, getStateOffset

key = "pyStateTest"
valueLen = 10
expected = b'0199956789'

# Check the full value
actual = getState(key, valueLen)
if actual != expected:
    msg = "Mismatch: actual {}, expected {})".format(actual, expected)
    print(msg)
    exit(1)

# Check a couple of segments
actualSegmentA = getStateOffset(key, valueLen, 0, 3)
expectedA = b'019'
if actualSegmentA != expectedA:
    msg = "Mismatched segment: actual {}, expected {})".format(actualSegmentA, expectedA)
    print(msg)
    exit(1)

actualSegmentB = getStateOffset(key, valueLen, 4, 4)
expectedB = b'9567'
if actualSegmentB != expectedB:
    msg = "Mismatched segment: actual {}, expected {})".format(actualSegmentB, expectedB)
    print(msg)
    exit(1)

print("Successful Python state reading check")