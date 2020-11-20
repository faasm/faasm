import os


def faasm_main():
    paths = [
        "/lib/python3.8/",
        "/lib/python3.8/site-packages/",
    ]

    print("CWD = {}".format(os.getcwd()))

    for i, p in enumerate(paths):
        print("\nPath {} - {}".format(i, p))
        dirlist = os.listdir(p)
        dirlist.sort()
        print(" ".join(["{}\n".format(d) for d in dirlist]))

    return 0
