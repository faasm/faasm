from os.path import exists
from os.path import join
from subprocess import call

from invoke import task

from tasks.util.env import PROJ_ROOT, FAASM_SYSROOT, LATEST_CMAKE


@task
def compile_onnx(ctx, clean=False):
    _do_compile_onnx_runtime(False)


@task
def compile_onnx_native(ctx):
    _do_compile_onnx_runtime(True)


def _do_compile_onnx_runtime(native):
    work_dir = join(PROJ_ROOT, "onnxruntime")

    if native:
        build_dir = join(work_dir, "build", "native")
        eigen_path = "/usr/local/include/eigen3"
    else:
        build_dir = join(work_dir, "build", "wasm")
        eigen_path = join(FAASM_SYSROOT, "include", "eigen3")

    # clean_dir(build_dir, clean)

    # See the script itself for more info on options (onnxruntime/tools/ci_build/build.py)
    # More docs in the repo too: https://github.com/microsoft/onnxruntime/blob/master/BUILD.md
    script_args = [
        "--cmake_path={}".format(LATEST_CMAKE),
        "--use_preinstalled_eigen",
        "--eigen_path={}".format(eigen_path),
        "--build_dir={}".format(build_dir),
        "--skip_onnx_tests",
        "--parallel",
    ]

    # Wasm-specific
    if not native:
        script_args.extend([
            "--wasm",
        ])

    # Check if we've already cloned the onnxruntime subprojects (and skip if so)
    cmake_submodule_checkout = join(PROJ_ROOT, "onnxruntime", "cmake", "external", "protobuf", "cmake")
    if exists(cmake_submodule_checkout):
        script_args.append("--skip_submodule_sync")
    else:
        call("git submodule update --init --recursive", cwd=work_dir)

    # Run the main build
    build_type = "RelWithDebInfo"
    build_cmd = [
        "python3", "tools/ci_build/build.py",
        "--update",
        "--build",
        "--config={}".format(build_type)
    ]

    if native:
        build_cmd.append("--build_shared_lib")

    build_cmd.extend(script_args)
    build_cmd_str = " ".join(build_cmd)
    print(build_cmd_str)
    res = call(build_cmd_str, shell=True, cwd=work_dir)
    if res != 0:
        print("Build command failed")
        exit(1)

    # Do the install
    make_dir = join(build_dir, build_type)
    call("make install", cwd=make_dir, shell=True)
