from os.path import exists
from os.path import join
from subprocess import check_output

from invoke import task

from tasks.util.env import THIRD_PARTY_DIR
from toolchain.python_env import WASM_HOST, BASE_CONFIG_CMD, WASM_CFLAGS, WASM_CXXFLAGS, WASM_LDFLAGS


@task
def compile_tf_lite(ctx):
    _do_compile_tf_lite(False)


@task
def compile_tf_lite_native(ctx):
    _do_compile_tf_lite(True)


def _do_compile_tf_lite(native):
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

    else:
        make_cmd = ["make -j 4"]
        make_cmd.extend(BASE_CONFIG_CMD)
        make_cmd.extend([
            "CFLAGS=\"{} -ftls-model=local-exec -ldlmalloc\"".format(WASM_CFLAGS),
            "CXXFLAGS=\"{} -ldlmalloc\"".format(WASM_CXXFLAGS),
            "LDFLAGS=\"{} -Xlinker --max-memory=1073741824\"".format(WASM_LDFLAGS),
            "MINIMAL_SRCS=",
            "TARGET={}".format(WASM_HOST),
            "BUILD_WITH_MMAP=false",
            "LIBS=\"-lstdc++ -ldlmalloc\"",
            "-C \"{}\"".format(tf_dir),
            "-f tensorflow/lite/tools/make/Makefile",
        ])

    check_output(" ".join(make_cmd), shell=True, cwd=tf_lite_dir)
