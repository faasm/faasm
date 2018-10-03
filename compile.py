import argparse

from compile.function import compile_function
from compile.libs import compile_lib
from compile.wasm2wast import wasm_to_wast

# Note: this should be run inside the wasm toolchain container

if __name__ == "__main__":
    top_parser = argparse.ArgumentParser()

    subparsers = top_parser.add_subparsers(dest='subparser_name')

    # Command to compile a user's function
    func_parser = subparsers.add_parser("func")
    func_parser.add_argument("user", help="Owner of the function")
    func_parser.add_argument("function", help="Function name")
    func_parser.add_argument("--libcurl", help="Link with libcurl", action="store_true")
    func_parser.add_argument("--debug", help="Include debug info", action="store_true")
    func_parser.add_argument("--cxx", help="Compile C++", action="store_true")

    # Command to compile a library
    lib_parser = subparsers.add_parser("lib")
    lib_parser.add_argument("name", help="Name of library to compile")

    # Command to compile WAST
    wast_parser = subparsers.add_parser("wast")
    wast_parser.add_argument("user", help="Owner of the function")
    wast_parser.add_argument("function", help="Function name")

    args = top_parser.parse_args()

    if args.subparser_name == "wast":
        wasm_to_wast(args)
    elif args.subparser_name == "func":
        compile_function(args)
    elif args.subparser_name == "lib":
        compile_lib(args)
    else:
        raise RuntimeError("Unrecognised command: ".format(args.subparser_name))
