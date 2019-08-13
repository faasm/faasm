from os.path import join, exists
from subprocess import check_output

from invoke import task

from tasks.util.codegen import find_codegen_binary
from tasks.util.env import PROJ_ROOT, WASM_DIR, HOME_DIR


@task
def run_codegen(ctx, dir_path):
    print("Running codegen for {}".format(dir_path))
    binary = find_codegen_binary()
    check_output("{} {}".format(binary, dir_path), shell=True)


@task
def run_wasm_codegen(ctx):
    dirs = ["demo", "errors", "python", "sgd"]
    for d in dirs:
        run_codegen(ctx, join(WASM_DIR, d))
