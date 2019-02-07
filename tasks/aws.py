from os import mkdir
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.env import FAASM_HOME

SDK_VERSION = "1.7.41"
RUNTIME_VERSION = "v0.1.0"

INSTALL_PATH = join(FAASM_HOME, "lambda")


@task
def build_aws_sdk(ctx):
    if not exists(INSTALL_PATH):
        mkdir(INSTALL_PATH)

    # AWS C++ SDK
    _set_up_cmake_project(
        "https://github.com/aws/aws-sdk-cpp.git",
        "aws-sdk-cpp",
        [
            "-DCMAKE_BUILD_TYPE=Release",
            "-DBUILD_SHARED_LIBS=OFF",
            '-DBUILD_ONLY="s3;sqs"',
            "-DCUSTOM_MEMORY_MANAGEMENT=OFF",
            "-DENABLE_UNITY_BUILD=ON",
            "-DENABLE_TESTING=OFF",
        ],
        SDK_VERSION
    )


@task
def build_lambda_runtime(ctx):
    if not exists(INSTALL_PATH):
        mkdir(INSTALL_PATH)

    # AWS C++ runtime
    _set_up_cmake_project(
        "https://github.com/awslabs/aws-lambda-cpp-runtime.git",
        "aws-lambda-cpp-runtime",
        [
            "-DCMAKE_BUILD_TYPE=Release",
            "-DBUILD_SHARED_LIBS=OFF",
        ],
        RUNTIME_VERSION
    )


def _set_up_cmake_project(repo_url, dir_name, cmake_args, version):
    checkout_path = join(FAASM_HOME, dir_name)

    if not exists(checkout_path):
        print("Cloning {}".format(repo_url))

        cmd = ["git", "clone", repo_url]
        call(" ".join(cmd), cwd=FAASM_HOME, shell=True)

        build_dir = join(checkout_path, "build")
        if not exists(build_dir):
            mkdir(build_dir)

        # Checkout the required version
        call("git checkout {}".format(version), cwd=build_dir, shell=True)

        cpp_sdk_build_cmd = [
            "cmake",
            "..",
            "-DCMAKE_INSTALL_PREFIX={}".format(INSTALL_PATH),
        ]

        cpp_sdk_build_cmd.extend(cmake_args)

        call(" ".join(cpp_sdk_build_cmd), cwd=build_dir, shell=True)
        call("make", cwd=build_dir, shell=True)
        call("make install", cwd=build_dir, shell=True)
