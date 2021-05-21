from os.path import join, abspath, dirname, isfile
import sys
import numpy as np
from subprocess import run, PIPE
from multiprocessing import cpu_count, Pool

PROJ_ROOT = dirname(dirname(abspath(__file__)))

BUILD_PATH = "/build/faasm/"

CONFIG_FILE = join(PROJ_ROOT, ".clang-tidy")
with open(CONFIG_FILE, "r") as fh:
    CONFIG = fh.read()

FILE_FILTERS = ["include/", "src/", "tests/"]
FILE_TYPES = [".cpp", ".h", ".c"]


def get_files(cmd):
    print("Running files cmd: {}".format(cmd))
    res = run(
        cmd,
        check=True,
        shell=True,
        cwd=PROJ_ROOT,
        stdout=PIPE,
        stderr=PIPE,
    )

    all_files = res.stdout.decode("utf-8").split("\n")
    all_files = [f.strip() for f in all_files if f.strip()]

    # Filter by dir
    folder_files = list()
    for d in FILE_FILTERS:
        folder_files.extend([f for f in all_files if f.startswith(d)])

    # Filter by type
    type_files = list()
    for t in FILE_TYPES:
        type_files.extend([f for f in folder_files if f.endswith(t)])

    print("Found {} files".format(len(type_files)))
    return type_files


def get_changed_files():
    cmd = "git diff --name-only master"
    changed_files = get_files(cmd)
    print("Running clang format on changed files:\n{}".format(changed_files))
    return changed_files


def get_all_files():
    cmd = "git ls-files"
    return get_files(cmd)


# Function to actually run clang-tidy
def do_tidy(file_list):
    if not file_list:
        print("Got no files to tidy")
        exit(1)

    n_cpus = cpu_count()
    file_chunks = np.array_split(file_list, n_cpus)
    non_empty_chunks = [f for f in file_chunks if len(f) > 0]
    n_non_empty = len(non_empty_chunks)

    print(
        "Running clang-tidy on {} files in {} chunks".format(
            len(file_list), n_non_empty
        )
    )

    with Pool(n_non_empty) as p:
        p.map(inner_tidy, non_empty_chunks)


def inner_tidy(file_chunk):
    print("Running clang-tidy on chunk of {} files".format(len(file_chunk)))
    cmd = [
        "clang-tidy-10",
        '-config "{}"'.format(CONFIG),
        "--fix",
        "--format-style=file",
        "-p={}".format(BUILD_PATH),
        " ".join(file_chunk),
    ]

    cmd = " ".join(cmd)
    run(cmd, check=True, shell=True, cwd=PROJ_ROOT)


if __name__ == "__main__":
    if len(sys.argv) == 2:
        argument = sys.argv[1]
        if isfile(argument):
            print("Running clang format on file {}".format(argument))
            file_list = [argument]

        elif argument == "ci":
            file_list = get_changed_files()

        else:
            print("Unrecognised argument")
            exit(1)
    else:
        print("Running clang format on full project {}".format(PROJ_ROOT))
        file_list = get_all_files()

    do_tidy(file_list)
