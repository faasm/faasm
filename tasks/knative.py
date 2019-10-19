import os
from os.path import join
from subprocess import call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import PROJ_ROOT
from tasks.util.files import clean_dir
from tasks.util.ibm import get_ibm_kubeconfig

K8S_DIR = join(PROJ_ROOT, "k8s")
BARE_METAL_CONF = join(K8S_DIR, "bare-metal")
BARE_METAL_REMOTE_CONF = join(K8S_DIR, "bare-metal-remote")
LOCAL_CONF = join(K8S_DIR, "local")
COMMON_CONF = join(K8S_DIR, "common")
IBM_CONF = join(K8S_DIR, "ibm")
LEGACY_CONF = join(K8S_DIR, "legacy")

#
# Notes on Knative client
# https://github.com/knative/client/blob/master/docs/cmd/kn_service_create.md
#
# Configuring the scheduler: https://knative.dev/docs/serving/configuring-the-autoscaler/
#

# TODO: Use the knative autoscaler rather than hard-coding a number of workers

FAASM_WORKER_ANNOTATIONS = [
    "autoscaling.knative.dev/enable-scale-to-zero=false",
    "autoscaling.knative.dev/stable-window=30s",
]

NATIVE_WORKER_ANNOTATIONS = [
    "autoscaling.knative.dev/enable-scale-to-zero=true",
    "autoscaling.knative.dev/stable-window=20s",
]

FAASM_WORKER_ARGS = [
    "--min-scale=10",  # Fixed number of workers (max one per host)
    "--max-scale=10",  # Max number of workers
    "--concurrency-limit=4",  # How many requests can be handled by a given worker
]

# Allow custom fixed numbers of workers for certain functions
# Expressed as (min, max)
NATIVE_WORKER_ARGS = {
    "reuters_svm": [
        "--min-scale=20",  # As each executes one thread, we can have multiple per machine
        "--max-scale=20",
        "--concurrency-limit=1",
    ],
    "default": [
        "--min-scale=2",
        "--max-scale=2",
        "--concurrency-limit=1",  # Native executors handle one request at a time
    ]
}

KNATIVE_FUNC_PREFIX = "faasm-"
NATIVE_WORKER_IMAGE_PREFIX = "faasm/knative-native-"

FAASM_WORKER_NAME = "{}worker".format(KNATIVE_FUNC_PREFIX)
FAASM_WORKER_IMAGE = "faasm/knative-worker"

ONE_MIN = 60000

KNATIVE_ENV = {
    "REDIS_STATE_HOST": "redis-state",
    "REDIS_QUEUE_HOST": "redis-queue",
    "HOST_TYPE": "knative",
    "LOG_LEVEL": "debug",
    "CGROUP_MODE": "off",
    "NETNS_MODE": "off",
    "MAX_IN_FLIGHT_RATIO": "1",
    "MAX_WORKERS_PER_FUNCTION": "4",  # This limit is per-host. We only want one instance per core
    "THREADS_PER_WORKER": "10",
    "FS_MODE": "on",
    "BOUND_TIMEOUT": str(2 * ONE_MIN),  # How long a bound worker sticks around for
    "UNBOUND_TIMEOUT": str(10 * ONE_MIN),  # How long an unbound worker sticks around for
    "GLOBAL_MESSAGE_TIMEOUT": str(2 * ONE_MIN),  # How long things wait for messages on global bus
}


def _fn_name(function):
    return "{}{}".format(KNATIVE_FUNC_PREFIX, function.replace("_", "-"))


def _native_image_name(function):
    return "{}{}".format(NATIVE_WORKER_IMAGE_PREFIX, function)


def _kubectl_apply(path, env=None):
    cmd = [
        "kubectl", "apply", "-f", path
    ]

    shell_env_dict = os.environ.copy()
    if env:
        shell_env_dict.update(env)

    cmd_str = " ".join(cmd)
    print(cmd_str)

    ret_code = call(cmd_str, shell=True, env=shell_env_dict)

    if ret_code != 0:
        raise RuntimeError("Command failed: {}".format(cmd_str))


@task
def delete_knative_worker(ctx):
    _delete_knative_fn("worker")


@task
def deploy_knative(ctx, local=False, bare_metal=False, ibm=False):
    if not local and not bare_metal and not ibm:
        print("Must provide one flag from local, bare-metal and ibm")
        return

    faasm_conf = get_faasm_config()

    shell_env = {}
    extra_env = {}
    if ibm:
        # IBM requires specifying custom kubeconfig
        shell_env["KUBECONFIG"] = get_ibm_kubeconfig()

        extra_env = {
            "FUNCTION_STORAGE": "ibm",
            "IBM_API_KEY": faasm_conf["IBM"]["api_key"],
        }
    elif bare_metal:
        extra_env = {
            "FUNCTION_STORAGE": "fileserver",
            "FILESERVER_URL": "http://upload:8002",
        }

    # State timings for knative environment
    extra_env.update({
        "STATE_STALE_THRESHOLD": "60000",
        "STATE_CLEAR_THRESHOLD": "300000",
        "STATE_PUSH_INTERVAL": "1000",
    })

    # Deploy the other K8s stuff (e.g. redis)
    _kubectl_apply(join(COMMON_CONF, "namespace.yml"), env=shell_env)
    _kubectl_apply(COMMON_CONF, env=shell_env)

    if local:
        _kubectl_apply(BARE_METAL_CONF)
        _kubectl_apply(LOCAL_CONF)
    elif bare_metal:
        _kubectl_apply(BARE_METAL_CONF)
        _kubectl_apply(BARE_METAL_REMOTE_CONF)

    _deploy_knative_fn(
        FAASM_WORKER_NAME,
        FAASM_WORKER_IMAGE,
        FAASM_WORKER_ANNOTATIONS,
        FAASM_WORKER_ARGS,
        extra_env=extra_env,
        shell_env=shell_env
    )


def _delete_knative_fn(name):
    cmd = [
        "kn", "service", "delete", _fn_name(name), "--namespace=faasm"
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)
    call(cmd_str, shell=True)


def _deploy_knative_fn(name, image, annotations, extra_args, extra_env=None, shell_env=None):
    cmd = [
        "kn", "service", "create", name,
        "--image", image,
        "--namespace", "faasm",
        "--force",
    ]

    cmd.extend(extra_args)

    # Add annotations
    for annotation in annotations:
        cmd.append("--annotation {}".format(annotation))

    # Add standard environment
    for key, value in KNATIVE_ENV.items():
        cmd.append("--env {}={}".format(key, value))

    # Add extra environment
    extra_env = extra_env if extra_env else {}
    for key, value in extra_env.items():
        cmd.append("--env {}={}".format(key, value))

    cmd_string = " ".join(cmd)
    print(cmd_string)

    shell_env_dict = os.environ.copy()
    if shell_env:
        shell_env_dict.update(shell_env)

    call(cmd_string, shell=True, env=shell_env_dict)


@task
def build_knative_native(ctx, user, function, host=False, clean=False, nopush=False):
    if host:
        build_dir = join(PROJ_ROOT, "build", "knative_native")
        target = "{}-knative".format(function)

        clean_dir(build_dir, clean)

        cmd = [
            "cmake",
            "-DCMAKE_CXX_COMPILER=/usr/bin/clang++",
            "-DCMAKE_C_COMPILER=/usr/bin/clang",
            "-DFAASM_BUILD_TYPE=knative-native",
            "-DCMAKE_BUILD_TYPE=Debug",
            "-DFAASM_AWS_SUPPORT=OFF",
            PROJ_ROOT
        ]
        call(" ".join(cmd), cwd=build_dir, shell=True)

        make_cmd = "cmake --build . --target {} -- -j".format(target)
        call(make_cmd, cwd=build_dir, shell=True)

    else:
        # Build the container
        tag_name = "{}{}".format(NATIVE_WORKER_IMAGE_PREFIX, function)
        cmd = [
            "docker",
            "build",
            "--no-cache" if clean else "",
            "-t", tag_name,
            "--build-arg", "FAASM_USER={}".format(user),
            "--build-arg", "FAASM_FUNC={}".format(function),
            "-f", "docker/knative-native.dockerfile",
            "."
        ]

        cmd_string = " ".join(cmd)
        print(cmd_string)
        res = call(cmd_string, shell=True, cwd=PROJ_ROOT)
        if res != 0:
            print("Building container failed")
            return 1

        # Push the container
        if not nopush:
            cmd = "docker push {}".format(tag_name)
            call(cmd, shell=True, cwd=PROJ_ROOT)


@task
def deploy_knative_native(ctx, user, function):
    faasm_config = get_faasm_config()

    if not faasm_config.has_section("Kubernetes"):
        print("Must have faasm config set up with kubernetes section")
        return 1

    # Host and port required for chaining native functions
    invoke_host = faasm_config["Kubernetes"]["invoke_host"]
    invoke_port = faasm_config["Kubernetes"]["invoke_port"]

    # Allow overriding func args for specific functions
    func_args = NATIVE_WORKER_ARGS.get(function)
    if not func_args:
        func_args = NATIVE_WORKER_ARGS["default"]

    _deploy_knative_fn(
        _fn_name(function),
        _native_image_name(function),
        NATIVE_WORKER_ANNOTATIONS,
        func_args,
        extra_env={
            "FAASM_INVOKE_HOST": invoke_host,
            "FAASM_INVOKE_PORT": invoke_port,
        }
    )


@task
def delete_knative_native(ctx, user, function):
    function = function.replace("_", "-")
    _delete_knative_fn(function)


@task
def knative_native_local(ctx, user, function):
    img_name = _native_image_name(function)
    cmd = [
        "docker", "run",
        "-p 8080:8080",
        "--env LOG_LEVEL=debug",
        "--env FAASM_INVOKE_HOST=127.0.0.1",
        "--env FAASM_INVOKE_PORT=8080",
        img_name
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, shell=True, cwd=PROJ_ROOT)
