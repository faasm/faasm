from os import mkdir
from os.path import exists, join
from shutil import rmtree
from subprocess import call

from compile.env import PROJ_ROOT, CXX, CC, COMPILER_FLAGS, ENV_DICT

BUILD_DIR = join(PROJ_ROOT, "work")


def compile_function(args):
    """
    Compiles the given function
    """

    print("Compiling {} for user {}".format(args.function, args.user))

    # Recreate the build dir
    if exists(BUILD_DIR):
        rmtree(BUILD_DIR)
    mkdir(BUILD_DIR)

    func_path = join("func", "function_{}.c".format(args.function))

    compile_cmd = [
        CXX if args.cxx else CC,
        *COMPILER_FLAGS,
        "-Oz",
        "-fvisibility=hidden",
        func_path,
        "-I", join("include", "faasm"),
        "-o", join("work", "out.wasm")
    ]

    # Debug
    if args.debug:
        compile_cmd.append("-g")

    # Add libcurl
    if args.libcurl:
        compile_cmd.append("-lcurl")

    compile_cmd_str = " ".join(compile_cmd)
    res = call(compile_cmd_str, shell=True, cwd=PROJ_ROOT)
    if res != 0:
        raise RuntimeError("Compile call failed")
