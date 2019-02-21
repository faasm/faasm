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
    print("Running Lambda build")

    # Compile the whole project passing in the right config
    build_dir = join(PROJ_ROOT, "lambda_fn_build")
    _build_cmake_project(build_dir, [
        "-DFAASM_BUILD_TYPE=lambda",
        "-DCMAKE_BUILD_TYPE=Release",
    ], clean=True)


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

    _do_upload(zip_file_path, func_name)


@task
def build_lambda_runtime(ctx):
    print("Running Lambda runtime build")

    # Compile the whole project passing in the right config
    build_dir = join(PROJ_ROOT, "lambda_build")
    _build_cmake_project(build_dir, [
        "-DFAASM_BUILD_TYPE=lambda-runtime",
        "-DCMAKE_BUILD_TYPE=Release",
    ], clean=False)

    # Create the zip
    cmake_zip_target = "aws-lambda-package-worker-lambda"
    call("make {}".format(cmake_zip_target), cwd=LAMBDA_BUILD_DIR, shell=True)


@task
def upload_lambda_runtime(ctx):
    zip_file_path = join(
        LAMBDA_BUILD_DIR,
        "src",
        "worker",
        "worker-lambda.zip"
    )

    redis_url = _get_elasticache_url()
    print("Using redis instance at {}".format(redis_url))

    mem = 1 * 128
    timeout = 15
    _do_upload(zip_file_path, "faasm-runtime", memory=mem, timeout=timeout, environment={
        "LOG_LEVEL": "debug",
        "SYSTEM_MODE": "aws",
        "CGROUP_MODE": "off",
        "NETNS_MODE": "off",
        "THREADS_PER_WORKER": "4",
        "BUCKET_NAME": "faasm-bucket",
        "QUEUE_NAME": "faasm-messages",
        "REDIS_STATE_HOST": redis_url,
        "REDIS_QUEUE_HOST": redis_url,
    })


def _get_elasticache_url():
    client = boto3.client("elasticache", region_name=AWS_REGION)

    response = client.describe_cache_clusters(
        CacheClusterId="faasm-redis",
        ShowCacheNodeInfo=True,
    )

    cluster_data = response["CacheClusters"][0]
    node_data = cluster_data["CacheNodes"][0]

    return node_data["Endpoint"]["Address"]


def _do_upload(zip_file_path, func_name, memory=128, timeout=15, environment=None):
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

        update_args = {
            "Runtime": "provided",
            "Role": "arn:aws:iam::{}:role/{}".format(AWS_ACCOUNT_ID, AWS_LAMBDA_ROLE),
            "Handler": func_name,
            "Code": {"ZipFile": content},
            "MemorySize": memory,
            "Timeout": timeout,
        }

        if environment:
            lambda_env = {
                "Variables": environment
            }
            update_args["Environment"] = lambda_env

        kwargs.update(update_args)

        client.create_function(**kwargs)


#
# @task
# def build_aws_sdk(ctx):
#     """
#     Builds AWS SDK for use by Lambda functions themselves
#     """
#
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


# @task
# def build_lambda_runtime(ctx):
#     """
#     Builds AWS Lambda runtime itself
#     """
#
#     if not exists(INSTALL_PATH):
#         mkdir(INSTALL_PATH)
#
#     # AWS C++ runtime
#     # NOTE: building in debug mode enables verbose logging by default
#     # Verbosity can be set on release builds (1 = info, 2 = debug, 3 = all)
#     _set_up_cmake_project(
#         "https://github.com/awslabs/aws-lambda-cpp-runtime.git",
#         "aws-lambda-cpp-runtime",
#         [
#             "-DCMAKE_BUILD_TYPE=Release",
#             "-DLOG_VERBOSITY=1",
#             "-DBUILD_SHARED_LIBS=OFF",
#         ],
#         RUNTIME_VERSION,
#         clean=True
#     )


def _set_up_cmake_project(repo_url, dir_name, cmake_args, version, clean=False):
    checkout_path = join(FAASM_HOME, dir_name)

    if not exists(checkout_path):
        print("Cloning {}".format(repo_url))

        cmd = ["git", "clone", repo_url, dir_name]
        call(" ".join(cmd), cwd=FAASM_HOME, shell=True)

        # Checkout the required version
        call("git checkout {}".format(version), cwd=checkout_path, shell=True)

    build_dir = join(checkout_path, "build")
    _build_cmake_project(build_dir, cmake_args, clean=clean)

    call("make install", cwd=build_dir, shell=True)


def _build_cmake_project(build_dir, cmake_args, clean=False):
    # Nuke if required
    if clean and exists(build_dir):
        rmtree(build_dir)

    # Create build dir if necessary
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
