import ctypes
from os import RTLD_LAZY
from os.path import join

NATIVE_LIB_DIR = "/usr/local/faasm/install/lib/"
WASM_LIB_DIR = "/lib/"

NATIVE_LIBFAASM = join(NATIVE_LIB_DIR, "libfaasm.so")
WASM_LIBFAASM = join(WASM_LIB_DIR, "libfaasm.so")

NATIVE_LIBEMULATOR = join(NATIVE_LIB_DIR, "libemulator.so")

IS_NATIVE = True


def main():
    # Load emulator if in native mode
    if IS_NATIVE:
        ctypes.CDLL(NATIVE_LIBEMULATOR)
        clib = ctypes.CDLL(NATIVE_LIBFAASM, mode=RTLD_LAZY)
    else:
        clib = ctypes.CDLL(WASM_LIBFAASM, mode=RTLD_LAZY)

    input_size = clib.faasmGetInputSize()

    print("Input size: {}\n".format(input_size))


if __name__ == "__main__":
    main()
