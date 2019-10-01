from os.path import join, exists

from tasks.util.env import PROJ_ROOT, HOME_DIR


def _do_find_codegen(bin_name):
    possible_binaries = [
        "/faasm/build/bin/{}".format(bin_name),  # Containers
        join(PROJ_ROOT, "cmake-build-debug/bin/{}".format(bin_name)),  # CLion
    ]

    existing_binaries = [p for p in possible_binaries if exists(p)]
    if not existing_binaries:
        print("Could not find any codegen binaries (options = {})".format(possible_binaries))
        exit(1)

    binary = existing_binaries[0]
    if len(existing_binaries) > 1:
        print("WARNING: found multiple codegen binaries, taking {}".format(binary))

    return binary


def find_codegen_shared_lib():
    return _do_find_codegen("codegen_shared_obj")


def find_codegen_func():
    return _do_find_codegen("codegen_func")
