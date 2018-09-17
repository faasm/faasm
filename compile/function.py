from os import mkdir
from os.path import exists, join
from shutil import rmtree
from subprocess import call

import requests

from compile.env import PROJ_ROOT, CXX, CC, CXXFLAGS, CFLAGS, WASM_LIB_DIR

BUILD_DIR = "/tmp/faasm"


def compile_function(args):
    """
    Compiles a user's function
    """

    print("Compiling {} for user {}".format(args.function, args.user))

    if exists(BUILD_DIR):
        rmtree(BUILD_DIR)
    mkdir(BUILD_DIR)

    func_dir = join(PROJ_ROOT, "wasm", args.user, args.function)
    if exists(func_dir):
        rmtree(func_dir)
    mkdir(func_dir)

    func_path = join(PROJ_ROOT, "func", "function_{}.c".format(args.function))
    if not exists(func_path):
        print("Could not find function at {}".format(func_path))
        exit(1)

    compile_cmd = [
        CXX if args.cxx else CC,
        CXXFLAGS if args.cxx else CFLAGS,
        func_path,
        "-I", join(PROJ_ROOT, "include", "faasm")
    ]

    # Debug
    if args.debug:
        compile_cmd.append("-g")

    # Add libcurl
    if args.libcurl:
        compile_cmd.extend([
            "-I", join(WASM_LIB_DIR, "libcurl", "include"),
            join(WASM_LIB_DIR, "libcurl", "lib", "libcurl.a")
        ])

    compile_cmd_str = " ".join(compile_cmd)
    print("Calling: {}".format(compile_cmd_str))
    res = call(compile_cmd_str, shell=True, cwd=BUILD_DIR)
    if res != 0:
        raise RuntimeError("Compile call failed")

    # Set up function directory
    user_dir = join(PROJ_ROOT, "wasm", args.user)
    if not exists(user_dir):
        mkdir(user_dir)
    if not exists(func_dir):
        mkdir(func_dir)

    # Put function file in place
    wasm_file = join(BUILD_DIR, "a.out")
    dest_file = join(func_dir, "function.wasm")

    print("Putting {} in place at {}.".format(wasm_file, dest_file))
    call(["cp", wasm_file, dest_file])

    # Upload to Faasm
    url = "http://localhost:8080/f/{}/{}/".format(args.user, args.function)

    print("Uploading file to {}".format(url))
    with open(wasm_file, "rb") as fh:
        # Read in the whole file
        data = fh.read()
        response = requests.put(url, data=data, headers={'Content-Type': 'application/octet-stream'})

    print(response.content)
