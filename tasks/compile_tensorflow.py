from os.path import exists
from os.path import join
from shutil import rmtree
from subprocess import check_output, call

from invoke import task

from tasks.util.env import THIRD_PARTY_DIR, PROJ_ROOT
from toolchain.python_env import WASM_HOST, BASE_CONFIG_CMD, WASM_CFLAGS, WASM_CXXFLAGS, WASM_LDFLAGS


@task
def compile_tf_lite(ctx, clean=False):
    _do_compile_tf_lite(False, clean=clean)


@task
def compile_tf_lite_native(ctx, clean=False):
    _do_compile_tf_lite(True, clean=clean)


def _do_compile_tf_lite(native, clean=False):
    tf_dir = join(THIRD_PARTY_DIR, "tensorflow")
    tf_lite_dir = join(tf_dir, "tensorflow", "lite")
    tf_make_dir = join(tf_lite_dir, "tools", "make")

    download_check_dir = join(tf_make_dir, "downloads", "absl")
    if not exists(download_check_dir):
        download_script = join(tf_make_dir, "download_dependencies.sh")
        check_output(download_script, shell=True)

    if native:
        make_cmd = [
            "make -j 4",
            "VERBOSE=1",
            "MINIMAL_SRCS=",
            "BUILD_WITH_NNAPI=OFF",
            "-C {}".format(tf_dir),
            "-f tensorflow/lite/tools/make/Makefile",
        ]

        clean_dir = join(tf_make_dir, "gen", "linux_x86_64")

    else:
        make_cmd = ["make -j 4"]
        make_cmd.extend(BASE_CONFIG_CMD)
        make_cmd.extend([
            "CFLAGS=\"{} -ftls-model=local-exec -ldlmalloc\"".format(WASM_CFLAGS),
            "CXXFLAGS=\"{} -ldlmalloc\"".format(WASM_CXXFLAGS),
            "LDFLAGS=\"{} -Xlinker --max-memory=4294967296\"".format(WASM_LDFLAGS),
            "MINIMAL_SRCS=",
            "TARGET={}".format(WASM_HOST),
            "BUILD_WITH_MMAP=false",
            "LIBS=\"-lstdc++ -ldlmalloc\"",
            "-C \"{}\"".format(tf_dir),
            "-f tensorflow/lite/tools/make/Makefile",
        ])

        clean_dir = join(tf_make_dir, "gen", "wasm32-unknown-none_x86_64")

    if clean and exists(clean_dir):
        rmtree(clean_dir)

    res = call(" ".join(make_cmd), shell=True, cwd=tf_lite_dir)
    if res != 0:
        raise RuntimeError("Failed to compile Tensorflow lite")
