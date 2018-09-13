# ----------------------------------
# Script to build faasm functions
# ----------------------------------

import argparse
from os import mkdir
from os.path import exists, dirname, realpath, join
import requests
from subprocess import call

PROJ_ROOT = dirname(dirname(realpath(__file__)))
BUILD_DIR = "/tmp/faasm"

EMCC = "/usr/local/code/emsdk/emscripten/1.38.10/emcc"

WAVIX_BUILD = "/usr/local/code/WavixBuild"
WAVIX_SYS = join(WAVIX_BUILD, "sys")
WAVIX_CLANG = join(WAVIX_BUILD, "bootstrap/bin/clang++")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("user", help="Owner of the function")
    parser.add_argument("function", help="Function name")
    parser.add_argument("--libcurl", help="Link with libcurl", action="store_true")
    parser.add_argument("--noopt", help="Turn off Emscripten optimizations", action="store_true")
    parser.add_argument("--debug", help="Include debug info", action="store_true")
    parser.add_argument("--wavix", help="Build with wavix", action="store_true")

    args = parser.parse_args()

    print("Compiling {} for user {}".format(args.function, args.user))

    if exists(BUILD_DIR):
        call(["rm", "-rf", BUILD_DIR])
    mkdir(BUILD_DIR)

    func_path = join(PROJ_ROOT, "func", "function_{}.c".format(args.function))
    if not exists(func_path):
        print("Could not find function at {}".format(func_path))
        exit(1)

    if args.wavix:
        compile_cmd = [
            WAVIX_CLANG,
            "--target", "wasm32-unknown-wavix",
            "--sysroot", WAVIX_SYS
        ]
    else:
        compile_cmd = [
            EMCC,
            "-s", "WASM=1",
            "-o", "function.js"
        ]

    compile_cmd.extend([
        func_path,
        "-I", join(PROJ_ROOT, "include", "faasm")
    ])

    # Add optimisations if necessary
    if not args.noopt:
        compile_cmd.append("-Oz")

    # Debug
    if args.debug:
        compile_cmd.append("-g")

    # Add libcurl
    if args.libcurl:
        compile_cmd.extend([
            "-I", join(PROJ_ROOT, "lib", "libcurl", "include"),
            join(PROJ_ROOT, "lib", "libcurl", "libcurl.so")
        ])

    print("Calling: {}".format(" ".join(compile_cmd)))
    call(compile_cmd, cwd=BUILD_DIR)

    # Set up function directory
    user_dir = join(PROJ_ROOT, "wasm", args.user)
    func_dir = join(PROJ_ROOT, "wasm", args.user, args.function)
    if not exists(user_dir):
        mkdir(user_dir)
    if not exists(func_dir):
        mkdir(func_dir)

    # Put function file in place
    wasm_file = join(BUILD_DIR, "function.wasm")
    call(["cp", wasm_file, func_dir])

    # Upload to Faasm
    url = "http://localhost:8080/f/{}/{}/".format(args.user, args.function)

    print("Uploading file to {}".format(url))
    with open(wasm_file, "rb") as fh:
        # Read in the whole file
        data = fh.read()
        response = requests.put(url, data=data, headers={'Content-Type': 'application/octet-stream'})

    print(response.content)
