import os
from os.path import join, exists
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

FAASM_WORKER_ANNOTATIONS = [
    "autoscaling.knative.dev/class=kpa.autoscaling.knative.dev",
    "autoscaling.knative.dev/metric=concurrency",
    "autoscaling.knative.dev/minScale=1",  # Always have one pod running
    "autoscaling.knative.dev/maxScale=10",  # Max pods
]

NATIVE_WORKER_ANNOTATIONS = [
    "autoscaling.knative.dev/class=kpa.autoscaling.knative.dev",
    "autoscaling.knative.dev/metric=concurrency",
    "autoscaling.knative.dev/minScale=0",  # Scale from zero
    "autoscaling.knative.dev/maxScale=10",  # Max pods
]

FAASM_WORKER_ARGS = [
    "--concurrency-limit=4",  # Hard limit on number of requests to be handled by a single replica
    "--concurrency-target=4",  # Recommended scale-out (defaults to concurrency-limit)
]

NATIVE_WORKER_ARGS = [
    "--concurrency-limit=1",  # Native executors handle one request at a time
]

KNATIVE_FUNC_PREFIX = "faasm-"
NATIVE_WORKER_IMAGE_PREFIX = "faasm/knative-native-"

FAASM_WORKER_NAME = "{}worker".format(KNATIVE_FUNC_PREFIX)
FAASM_WORKER_IMAGE = "faasm/knative-worker"

KNATIVE_ENV = {
    "REDIS_STATE_HOST": "redis-state",
    "REDIS_QUEUE_HOST": "redis-queue",
    "FUNCTION_STORAGE": "fileserver",
    "HOST_TYPE": "knative",
    "LOG_LEVEL": "debug",
    "CGROUP_MODE": "off",
    "NETNS_MODE": "off",
    "THREADS_PER_WORKER": "10",
    "MAX_QUEUE_RATIO": "1",
    "MAX_WORKERS_PER_FUNCTION": "10",
    "FS_MODE": "on",
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
def k8s_delete_worker(ctx):
    _delete_knative_fn("worker")


@task
def k8s_deploy(ctx, local=False, bare_metal=False, ibm=False):
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
            "FILESERVER_URL": "http://upload:8002"
        }

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

    _deploy_knative_fn(
        _fn_name(function),
        _native_image_name(function),
        NATIVE_WORKER_ANNOTATIONS,
        NATIVE_WORKER_ARGS,
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
