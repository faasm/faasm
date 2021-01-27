import hashlib


def faasm_main():
    expected_hash = b"\x87 1j\x1a\xfe\xd8\x1c\x1d\xd0\x02d\x03\xaa\xa2["
    hash_result = hashlib.md5(b"Hello faasm")
    actual_hash = hash_result.digest()

    if actual_hash != expected_hash:
        print("Actual hash and expected hash not equal")
        print("Actual = {}, expected = {}".format(actual_hash, expected_hash))
        return 1

    return 0
