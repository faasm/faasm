from os import mkdir
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.util.env import PROJ_ROOT
from tasks.util.typescript import ASC_BINARY

ONNXJS_DIR = join(PROJ_ROOT, "onnxjs")


@task
def compile_onnxjs(ctx):
    build_dir = join(ONNXJS_DIR, "build")
    if not exists(build_dir):
        mkdir(build_dir)

    asc_binary = join(ONNXJS_DIR, "node_modules", "assemblyscript", "bin", "asc")

    cmd = [
        asc_binary,
        "lib/api/inference-session.ts",
        "-b build/inference-session.wasm"
    ]

    cmd_string = " ".join(cmd)
    call(cmd_string, cwd=ONNXJS_DIR, shell=True)
