# ----------------------------------
# Script to build faasm functions
# ----------------------------------

import argparse
from os import mkdir
from os.path import exists, dirname, realpath, join
from subprocess import call

PROJ_ROOT = dirname(dirname(realpath(__file__)))
BUILD_DIR = "/tmp/faasm"
EMCC = "/usr/local/code/emsdk/emscripten/1.38.10/emcc"

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("user", help="Owner of the function")
    parser.add_argument("function", help="Function name")
    parser.add_argument("--libcurl", help="Link with libcurl", action="store_true")
    parser.add_argument("--noopt", help="Turn off Emscripten optimizations", action="store_true")
    parser.add_argument("--wast", help="Output wast", action="store_true")

    args = parser.parse_args()

    print("Compiling {} for user {}".format(args.function, args.user))

    if exists(BUILD_DIR):
        call(["rm", "-rf", BUILD_DIR])
    mkdir(BUILD_DIR)

    func_path = join(PROJ_ROOT, "func", "function_{}.c".format(args.function))
    if not exists(func_path):
        print("Could not find function at {}".format(func_path))
        exit(1)

    build_cmd = [
        EMCC,
        func_path,
        "-s", "WASM=1",
        "-o", "function.js",
        "-I", join(PROJ_ROOT, "include", "faasm")
    ]

    # Debug
    if args.wast:
        build_cmd.append("-g")

    # Add optimisations if necessary
    if not args.noopt:
        build_cmd.append("-Oz")

    # Add libcurl
    if args.libcurl:
        build_cmd.extend([
            "-I", join(PROJ_ROOT, "lib", "libcurl", "include"),
            join(PROJ_ROOT, "lib", "libcurl", "libcurl.so")
        ])

    print("Calling: {}".format(" ".join(build_cmd)))
    call(build_cmd, cwd=BUILD_DIR)

    # Set up function directory
    user_dir = join(PROJ_ROOT, "wasm", args.user)
    func_dir = join(PROJ_ROOT, "wasm", args.user, args.function)
    if not exists(user_dir):
        mkdir(user_dir)
    if not exists(func_dir):
        mkdir(func_dir)

    # Put function file in place
    call(["cp", join(BUILD_DIR, "function.wasm"), func_dir])
    if args.wast:
        call(["cp", join(BUILD_DIR, "function.wast"), func_dir])
