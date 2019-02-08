from os import mkdir
from os.path import join, exists
from shutil import rmtree
from subprocess import call

import boto3
from botocore.exceptions import ClientError
from invoke import task

from tasks.env import FAASM_HOME, PROJ_ROOT

SDK_VERSION = "1.7.41"
RUNTIME_VERSION = "master"

INSTALL_PATH = join(FAASM_HOME, "lambda")
LAMBDA_BUILD_DIR = join(PROJ_ROOT, "lambda_build")

# TODO - avoid hard-coding
AWS_ACCOUNT_ID = "733781933474"
AWS_LAMBDA_ROLE = "faasm-lambda-role"
AWS_REGION = "eu-west-1"


@task
def build_lambdas(ctx):
    # Compile the whole project passing in the right config
    print("Running Lambda build")

    build_dir = join(PROJ_ROOT, "lambda_build")

    rmtree(build_dir)

    _build_cmake_project(build_dir, [
        "-DFAASM_BUILD_TYPE=lambda",
        "-DCMAKE_BUILD_TYPE=Debug",
    ])


@task
def upload_lambda_function(ctx, user, func_name):
    # Create the zip
    cmake_zip_target = "aws-lambda-package-{}-lambda".format(func_name)
    call("make {}".format(cmake_zip_target), cwd=LAMBDA_BUILD_DIR, shell=True)

    zip_file_path = join(
        LAMBDA_BUILD_DIR,
        "func",
        user,
        "{}-lambda.zip".format(func_name)
    )

    assert exists(zip_file_path), "Expected zip file at {}".format(zip_file_path)

    with open(zip_file_path, "rb") as fh:
        content = fh.read()

    # Upload the function
    client = boto3.client("lambda", region_name=AWS_REGION)

    # Check if function exists
    is_existing = True
    try:
        client.get_function(
            FunctionName=func_name,
        )
    except ClientError:
        is_existing = False

    kwargs = {
        "FunctionName": func_name,
    }

    if is_existing:
        print("{} already exists, updating".format(func_name))

        kwargs.update({
            "ZipFile": content,
        })

        client.update_function_code(**kwargs)
    else:
        print("{} does not already exist, creating".format(func_name))

        kwargs.update({
            "Runtime": "provided",
            "Role": "arn:aws:iam::{}:role/{}".format(AWS_ACCOUNT_ID, AWS_LAMBDA_ROLE),
            "Handler": func_name,
            "Code": {"ZipFile": content},
            "MemorySize": 128,
            "Timeout": 15,
        })

        client.create_function(**kwargs)


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
    # NOTE: building in debug mode enables verbose logging
    _set_up_cmake_project(
        "https://github.com/awslabs/aws-lambda-cpp-runtime.git",
        "aws-lambda-cpp-runtime",
        [
            "-DCMAKE_BUILD_TYPE=Debug",
            "-DLOG_VERBOSITY=3",
            "-DBUILD_SHARED_LIBS=OFF",
        ],
        RUNTIME_VERSION
    )


def _set_up_cmake_project(repo_url, dir_name, cmake_args, version):
    checkout_path = join(FAASM_HOME, dir_name)

    if not exists(checkout_path):
        print("Cloning {}".format(repo_url))

        cmd = ["git", "clone", repo_url, dir_name]
        call(" ".join(cmd), cwd=FAASM_HOME, shell=True)

        # Checkout the required version
        call("git checkout {}".format(version), cwd=checkout_path, shell=True)

    build_dir = join(checkout_path, "build")
    if exists(build_dir):
        rmtree(build_dir)

    mkdir(build_dir)

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
