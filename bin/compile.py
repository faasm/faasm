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
    parser.add_argument("--debug", help="Include debug info", action="store_true")
    parser.add_argument("--native", help="Compile for local machine", action="store_true")

    args = parser.parse_args()

    print("Compiling {} for user {}".format(args.function, args.user))

    if exists(BUILD_DIR):
        call(["rm", "-rf", BUILD_DIR])
    mkdir(BUILD_DIR)

    func_path = join(PROJ_ROOT, "func", "function_{}.c".format(args.function))
    if not exists(func_path):
        print("Could not find function at {}".format(func_path))
        exit(1)

    # Build basic compile command
    if args.native:
        # Native compiler along with fake emscripten header
        compile_cmd = [
            "clang",
            join(PROJ_ROOT, "func", "native_runner.c"),
            func_path,
            "-I", join(PROJ_ROOT, "include", "emscripten"),
            "-Wall",
            "-v"
        ]
    else:
        compile_cmd = [
            EMCC,
            func_path,
            "-s", "WASM=1",
            "-o", "function.js",
        ]

        # Add optimisations if necessary
        if not args.noopt:
            compile_cmd.append("-Oz")

    # Faasm header
    compile_cmd.extend(["-I", join(PROJ_ROOT, "include", "faasm")])

    # Debug
    if args.debug:
        compile_cmd.append("-g")

    # Add libcurl
    if args.libcurl and args.native:
        compile_cmd.append("-lcurl")
    elif args.libcurl:
        compile_cmd.extend([
            "-I", join(PROJ_ROOT, "lib", "libcurl", "include"),
            join(PROJ_ROOT, "lib", "libcurl", "libcurl.so")
        ])

    print("Calling: {}".format(" ".join(compile_cmd)))
    call(compile_cmd, cwd=BUILD_DIR)

    # Finished with native build now
    if args.native:
        exit(0)

    # Set up function directory
    user_dir = join(PROJ_ROOT, "wasm", args.user)
    func_dir = join(PROJ_ROOT, "wasm", args.user, args.function)
    if not exists(user_dir):
        mkdir(user_dir)
    if not exists(func_dir):
        mkdir(func_dir)

    # Put function file in place
    call(["cp", join(BUILD_DIR, "function.wasm"), func_dir])
    if args.debug:
        call(["cp", join(BUILD_DIR, "function.wast"), func_dir])
