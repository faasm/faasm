from os import mkdir
from os.path import join, exists
from shutil import rmtree
from subprocess import call

import boto3
from invoke import task

from tasks.env import FAASM_HOME, PROJ_ROOT

# SDK_VERSION = "1.7.41"
RUNTIME_VERSION = "v0.1.0"

INSTALL_PATH = join(FAASM_HOME, "lambda")
FUNC_BUILD_DIR = join(PROJ_ROOT, "func", "lambda_build")
LAMBDA_BUILD_DIR = join(PROJ_ROOT, "lambda", "build")

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_LAMBDA_ROLE = "faasm-lambda-role"
LAMBDA_FUNCTION_NAME = "faasm-lambda"
AWS_REGION = "eu-west-1"


@task
def build_lambdas(ctx):
    # Compile the whole project passing in the right config
    print("Running Lambda build")

    build_dir = join(PROJ_ROOT, "lambda_build")

    _build_cmake_project(build_dir, [
        "-DCMAKE_BUILD_TYPE=lambda",
    ])


@task
def upload_lambda_function(ctx, func_name):
    # Create the zip
    cmake_zip_target = "aws-lambda-package-{}".format(func_name)

    call("make {}".format(cmake_zip_target), cwd=FUNC_BUILD_DIR, shell=True)
    zip_file_path = join(FUNC_BUILD_DIR, "{}.zip".format(func_name))
    assert exists(zip_file_path), "Expected zip file at {}".format(zip_file_path)

    with open(zip_file_path, "rb") as content_file:
        content = content_file.read()

    # Upload the function
    client = boto3.client("lambda", region_name=AWS_REGION)

    kwargs = {
        "FunctionName": LAMBDA_FUNCTION_NAME,
        "Role": "arn:aws:iam::{}:role/{}".format(AWS_ACCOUNT_ID, AWS_LAMBDA_ROLE),
        "Handler": LAMBDA_FUNCTION_NAME,
        "Code": {"ZipFile": content},
        "Runtime": "provided",
        "MemorySize": 128,
        "Timeout": 15,
    }

    client.create_function(**kwargs)


# @task
# def build_aws_sdk(ctx):
#     if not exists(INSTALL_PATH):
#         mkdir(INSTALL_PATH)
#
#     # AWS C++ SDK
#     _set_up_cmake_project(
#         "https://github.com/aws/aws-sdk-cpp.git",
#         "aws-sdk-cpp",
#         [
#             "-DCMAKE_BUILD_TYPE=Release",
#             "-DBUILD_SHARED_LIBS=OFF",
#             '-DBUILD_ONLY="s3;sqs"',
#             "-DCUSTOM_MEMORY_MANAGEMENT=OFF",
#             "-DENABLE_UNITY_BUILD=ON",
#             "-DENABLE_TESTING=OFF",
#         ],
#         SDK_VERSION
#     )


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

        _build_cmake_project(build_dir, cmake_args)

        call("make install", cwd=build_dir, shell=True)


def _build_cmake_project(build_dir, cmake_args):
    if not exists(build_dir):
        mkdir(build_dir)

    cpp_sdk_build_cmd = [
        "cmake",
        "..",
        "-DCMAKE_INSTALL_PREFIX={}".format(INSTALL_PATH),
    ]

    cpp_sdk_build_cmd.extend(cmake_args)

    call(" ".join(cpp_sdk_build_cmd), cwd=build_dir, shell=True)
    call("make", cwd=build_dir, shell=True)
