from os.path import join, exists
from subprocess import check_output

from invoke import task

from tasks.util.env import PROJ_ROOT, WASM_DIR


@task
def run_codegen(ctx, dir_path):
    print("Running codegen for {}".format(dir_path))

    possible_binaries = [
        "cmake-build-release/bin/codegen",
        "cmake-build-debug/bin/codegen",
        "build/bin/codegen",
        "bin/codegen",
    ]
    possible_binaries = [join(PROJ_ROOT, p) for p in possible_binaries]

    existing_binaries = [p for p in possible_binaries if exists(p)]
    if not existing_binaries:
        print("Could not find any codegen binaries (options = {})".format(possible_binaries))
        exit(1)

    binary = existing_binaries[0]
    if len(existing_binaries) > 1:
        print("WARNING: found multiple codegen binaries, taking {}".format(binary))

    check_output("{} {}".format(binary, dir_path), shell=True)


@task
def run_wasm_codegen(ctx):
    dirs = ["demo", "errors", "python", "sgd"]
    for d in dirs:
        run_codegen(ctx, join(WASM_DIR, d))
