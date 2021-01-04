from ctypes import cdll


def main():
    loaded = cdll.LoadLibrary("libfakeLibA.so")
    print(loaded)


if __name__ == "__main__":
    main()
