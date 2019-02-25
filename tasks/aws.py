from os import mkdir
from os.path import join, exists
from shutil import rmtree
from subprocess import call

import boto3
from botocore.exceptions import ClientError
from invoke import task

from tasks.env import FAASM_HOME, PROJ_ROOT, RUNTIME_S3_BUCKET, AWS_REGION, AWS_ACCOUNT_ID

SDK_VERSION = "1.7.41"
RUNTIME_VERSION = "master"

INSTALL_PATH = join(FAASM_HOME, "lambda")

AWS_LAMBDA_ROLE = "faasm-lambda-role"


@task
def build_lambda_func(ctx, user, func):
    # Run the build
    build_dir = _build_lambda("func", target="{}-lambda".format(func))

    # Create the function zip
    _create_lambda_zip(func, build_dir)

    zip_file_path = join(
        build_dir,
        "func",
        user,
        "{}-lambda.zip".format(func)
    )

    assert exists(zip_file_path), "Could not find lambda zip at {}".format(zip_file_path)

    s3_key = _get_s3_key("{}-{}".format(user, func))
    _upload_lambda_to_s3(s3_key, zip_file_path)


@task
def build_lambda_redis(ctx):
    _build_system_lambda("redis")


@task
def build_lambda_worker(ctx):
    _build_system_lambda("worker")


@task
def build_lambda_dispatch(ctx):
    _build_system_lambda("dispatch")


@task
def build_lambda_codegen(ctx):
    _build_system_lambda("codegen")


def _build_system_lambda(module_name):
    build_dir = _build_lambda(module_name)
    _create_lambda_zip(module_name, build_dir)

    zip_file_path = join(
        build_dir,
        "src",
        module_name,
        "{}-lambda.zip".format(module_name)
    )

    assert exists(zip_file_path), "Could not find lambda zip at {}".format(zip_file_path)

    s3_key = _get_s3_key(module_name)
    _upload_lambda_to_s3(s3_key, zip_file_path)


def _build_lambda(module_name, target=None):
    print("Running Lambda {} build".format(module_name))

    # Compile the whole project passing in the right config
    build_dir = join(PROJ_ROOT, "lambda_{}_build".format(module_name))

    _build_cmake_project(build_dir, [
        "-DFAASM_BUILD_TYPE=lambda-{}".format(module_name),
        "-DCMAKE_BUILD_TYPE=Release",
    ], clean=False, target=target)

    return build_dir


def _create_lambda_zip(module_name, build_dir):
    cmake_zip_target = "aws-lambda-package-{}-lambda".format(module_name)
    call("make {}".format(cmake_zip_target), cwd=build_dir, shell=True)


def _get_s3_key(module_name):
    s3_key = "{}-lambda.zip".format(module_name)
    return s3_key


def _upload_lambda_to_s3(s3_key, zip_file_path):
    print("Uploading lambda {} to S3".format(s3_key))
    s3 = boto3.resource('s3', region_name=AWS_REGION)
    s3.meta.client.upload_file(zip_file_path, RUNTIME_S3_BUCKET, s3_key)


@task
def upload_lambda_redis(ctx):
    s3_key = _get_s3_key("redis")

    _do_upload(
        "faasm-redis",
        s3_bucket=RUNTIME_S3_BUCKET,
        s3_key=s3_key,
        environment=_get_lambda_env()
    )


@task
def upload_lambda_worker(ctx):
    s3_key = _get_s3_key("worker")

    # Worker timeout should be less than the function timeout to give things time to shut down gracefully
    lambda_env = _get_lambda_env()
    lambda_env["LAMBDA_WORKER_TIMEOUT"] = "30"
    function_timeout = 40

    _do_upload(
        "faasm-worker",
        memory=256,
        timeout=function_timeout,
        s3_bucket=RUNTIME_S3_BUCKET,
        s3_key=s3_key,
        environment=lambda_env
    )


@task
def upload_lambda_codegen(ctx):
    s3_key = _get_s3_key("codegen")

    # Codegen can be slow and take up memory
    _do_upload(
        "faasm-codegen",
        timeout=60,
        memory=300,
        s3_bucket=RUNTIME_S3_BUCKET,
        s3_key=s3_key,
        environment=_get_lambda_env()
    )


@task
def upload_lambda_func(ctx, user, func):
    s3_key = _get_s3_key("{}-{}".format(user, func))

    _do_upload(
        "{}-{}".format(user, func),
        s3_bucket=RUNTIME_S3_BUCKET,
        s3_key=s3_key,
        environment=_get_lambda_env()
    )


@task
def upload_lambda_dispatch(ctx):
    s3_key = _get_s3_key("dispatch")

    _do_upload(
        "faasm-dispatch",
        timeout=20,
        s3_bucket=RUNTIME_S3_BUCKET,
        s3_key=s3_key,
        environment=_get_lambda_env()
    )


def _get_lambda_env():
    redis_url = _get_elasticache_url()
    env = {
        "FUNCTION_STORAGE": "s3",
        "HOST_TYPE": "lambda",
        "LOG_LEVEL": "debug",
        "CGROUP_MODE": "off",
        "NETNS_MODE": "off",
        "THREADS_PER_WORKER": "4",
        "BUCKET_NAME": "faasm-runtime",
        "QUEUE_NAME": "faasm-messages",
        "SERIALISATION": "proto",
        "REDIS_STATE_HOST": redis_url,
        "REDIS_QUEUE_HOST": redis_url,
        "NO_SCHEDULER": "1",
        "GLOBAL_MESSAGE_BUS": "redis",
        "AWS_LOG_LEVEL": "info",
    }

    return env


def _get_elasticache_url():
    client = boto3.client("elasticache", region_name=AWS_REGION)

    response = client.describe_cache_clusters(
        CacheClusterId="faasm-redis",
        ShowCacheNodeInfo=True,
    )

    cluster_data = response["CacheClusters"][0]
    node_data = cluster_data["CacheNodes"][0]

    url = node_data["Endpoint"]["Address"]
    print("Found elasticache at {}".format(url))

    return url


def _get_security_group_ids():
    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    vpc_id = _get_vpc_id()

    response = ec2.describe_security_groups(
        Filters=[{
            "Name": "vpc-id",
            "Values": [
                vpc_id
            ]
        }]
    )

    group_ids = [sg["GroupId"] for sg in response["SecurityGroups"]]

    print("Found security groups {}".format(group_ids))

    return group_ids


def _get_vpc_id():
    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    response = ec2.describe_vpcs(
        Filters=[{
            "Name": "tag:Name",
            "Values": [
                "faasm-vpc",
            ]
        }]
    )

    vpc_data = response['Vpcs']
    assert len(vpc_data) == 1, "Found {} default VPCs, expected 1".format(len(vpc_data))

    vpc_id = vpc_data[0]['VpcId']

    print("Found faasm VPC {}".format(vpc_id))

    return vpc_id


def _get_private_subnet_ids():
    vpc_id = _get_vpc_id()

    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    response = ec2.describe_subnets(
        Filters=[{
            "Name": "vpc-id",
            "Values": [
                vpc_id
            ]
        }, {
            "Name": "tag:Name",
            "Values": [
                "faasm-private"
            ]
        }]
    )

    subnet_data = response['Subnets']
    ids = [sn["SubnetId"] for sn in subnet_data]

    print("Found subnet IDs {}".format(ids))

    return ids


def _do_upload(func_name, memory=128, timeout=15, concurrency=10,
               environment=None, zip_file_path=None, s3_bucket=None, s3_key=None):
    assert zip_file_path or (s3_bucket and s3_key), "Must give either a zip file or S3 bucket and key"

    if zip_file_path:
        assert exists(zip_file_path), "Expected zip file at {}".format(zip_file_path)

    # Get subnet IDs and security groups
    subnet_ids = _get_private_subnet_ids()
    security_group_ids = _get_security_group_ids()

    # Check if function exists
    is_existing = True
    client = boto3.client("lambda", region_name=AWS_REGION)
    try:
        client.get_function(
            FunctionName=func_name,
        )
    except ClientError:
        is_existing = False

    kwargs = {
        "FunctionName": func_name,
    }

    content = None
    if zip_file_path:
        with open(zip_file_path, "rb") as fh:
            content = fh.read()

    if is_existing:
        print("{} already exists, updating".format(func_name))

        if zip_file_path:
            kwargs["ZipFile"] = content
        else:
            kwargs["S3Bucket"] = s3_bucket
            kwargs["S3Key"] = s3_key

        client.update_function_code(**kwargs)
    else:
        print("{} does not already exist, creating".format(func_name))

        kwargs.update({
            "Runtime": "provided",
            "Role": "arn:aws:iam::{}:role/{}".format(AWS_ACCOUNT_ID, AWS_LAMBDA_ROLE),
            "Handler": func_name,
            "MemorySize": memory,
            "Timeout": timeout,
            "VpcConfig": {
                "SubnetIds": subnet_ids,
                "SecurityGroupIds": security_group_ids
            }
        })

        if zip_file_path:
            kwargs["Code"] = {"ZipFile": content}
        else:
            kwargs["Code"] = {"S3Bucket": s3_bucket, "S3Key": s3_key}

        if environment:
            lambda_env = {
                "Variables": environment
            }
            kwargs["Environment"] = lambda_env

        client.create_function(**kwargs)

    # Set up concurrency
    client.put_function_concurrency(
        FunctionName=func_name,
        ReservedConcurrentExecutions=concurrency,
    )


def _build_cmake_project(build_dir, cmake_args, clean=False, target=None):
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

    if target:
        call("make {}".format(target), cwd=build_dir, shell=True)
    else:
        call("make", cwd=build_dir, shell=True)
