import os
from copy import copy
from os.path import join
from subprocess import call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import PROJ_ROOT, FUNC_DIR
from tasks.util.files import clean_dir
from tasks.util.ibm import get_ibm_kubeconfig
from tasks.util.kubernetes import get_kubernetes_host_port
from tasks.util.version import get_faasm_version

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

KNATIVE_FUNC_PREFIX = "faasm-"

NATIVE_WORKER_IMAGE_PREFIX = "faasm/knative-native-"

KNATIVE_NATIVE_PY_NAME = "{}python".format(KNATIVE_FUNC_PREFIX)
KNATIVE_NATIVE_PY_IMAGE = "faasm/knative-native-python"

FAASM_WORKER_NAME = "{}worker".format(KNATIVE_FUNC_PREFIX)
FAASM_WORKER_IMAGE = "faasm/knative-worker"

ONE_MIN = 60000
THIRTY_SECS = 30000

KNATIVE_ENV = {
    "REDIS_STATE_HOST": "redis-state",
    "REDIS_QUEUE_HOST": "redis-queue",
    "HOST_TYPE": "knative",
    "LOG_LEVEL": "trace",
    "CGROUP_MODE": "off",
    "NETNS_MODE": "off",
    "MAX_IN_FLIGHT_RATIO": "1",
    "MAX_WORKERS_PER_FUNCTION": "4",  # This limit is per-host. We only want one instance per core
    "THREADS_PER_WORKER": "100",  # This is how many threads are available in total per host (across all functions)
    "BOUND_TIMEOUT": str(THIRTY_SECS),  # How long a bound worker sticks around for
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
def delete_knative_worker(ctx, hard=False):
    # Clear redis queue
    flush_cmd = "kubectl exec -n faasm redis-queue -- redis-cli flushall"
    call(flush_cmd, shell=True)

    _delete_knative_fn("worker", hard)


@task
def deploy_knative(ctx, replicas, local=False, ibm=False):
    faasm_conf = get_faasm_config()

    shell_env = {}
    if ibm:
        # IBM requires specifying custom kubeconfig
        shell_env["KUBECONFIG"] = get_ibm_kubeconfig()

        extra_env = {
            "FUNCTION_STORAGE": "ibm",
            "IBM_API_KEY": faasm_conf["IBM"]["api_key"],
        }
    else:
        extra_env = {
            "FUNCTION_STORAGE": "fileserver",
            "FILESERVER_URL": "http://upload:8002",
        }

    # Deploy the other K8s stuff (e.g. redis)
    _kubectl_apply(join(COMMON_CONF, "namespace.yml"), env=shell_env)
    _kubectl_apply(COMMON_CONF, env=shell_env)

    if local:
        _kubectl_apply(BARE_METAL_CONF)
        _kubectl_apply(LOCAL_CONF)
    else:
        _kubectl_apply(BARE_METAL_CONF)
        _kubectl_apply(BARE_METAL_REMOTE_CONF)

    _deploy_knative_fn(
        FAASM_WORKER_NAME,
        FAASM_WORKER_IMAGE,
        replicas,
        4,
        FAASM_WORKER_ANNOTATIONS,
        extra_env=extra_env,
        shell_env=shell_env
    )


def _delete_knative_fn(name, hard):
    func_name = _fn_name(name)
    if hard:
        cmd = [
            "kn", "service", "delete", func_name, "--namespace=faasm"
        ]

        cmd_str = " ".join(cmd)
        print(cmd_str)
        call(cmd_str, shell=True)
    else:
        # Delete the pods (they'll respawn)
        label = "serving.knative.dev/service={}".format(func_name)
        cmd = "kubectl -n faasm delete pods -l {} --wait=false --now".format(label)
        call(cmd, shell=True)


def _deploy_knative_fn(name, image, replicas, concurrency, annotations, extra_env=None, shell_env=None):
    version = get_faasm_version()
    image = "{}:{}".format(image, version)

    cmd = [
        "kn", "service", "create", name,
        "--image", image,
        "--namespace", "faasm",
        "--force",
    ]

    cmd.extend({
        "--min-scale={}".format(replicas),
        "--max-scale={}".format(replicas),
        "--concurrency-limit={}".format(concurrency),
    })

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
        version = get_faasm_version()
        tag_name = "{}:{}".format(_native_image_name(function), version)
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
def deploy_knative_native(ctx, user, function, replicas):
    func_name = _fn_name(function)
    image_name = _native_image_name(function)
    _do_deploy_knative_native(func_name, image_name, replicas)


@task
def deploy_knative_native_python(ctx, replicas):
    func_name = KNATIVE_NATIVE_PY_NAME
    image_name = KNATIVE_NATIVE_PY_IMAGE
    _do_deploy_knative_native(func_name, image_name, replicas)


def _do_deploy_knative_native(func_name, image_name, replicas):
    faasm_config = get_faasm_config()
    if not faasm_config.has_section("Kubernetes"):
        print("Must have faasm config set up with kubernetes section")
        return 1

    version = get_faasm_version()
    image_name = "{}:{}".format(image_name, version)

    # Host and port required for chaining native functions
    invoke_host, invoke_port = get_kubernetes_host_port()

    _deploy_knative_fn(
        func_name,
        image_name,
        replicas,
        1,
        NATIVE_WORKER_ANNOTATIONS,
        extra_env={
            "COLD_START_DELAY_MS": "500",
            "FAASM_INVOKE_HOST": invoke_host,
            "FAASM_INVOKE_PORT": invoke_port,
        },
    )


@task
def delete_knative_native(ctx, user, function, hard=False):
    _delete_knative_fn(function, hard)


@task
def delete_knative_native_python(ctx, hard=False):
    _delete_knative_fn("python", hard)


@task
def knative_native_local(ctx, user, function):
    img_name = _native_image_name(function)
    _do_knative_native_local(img_name)


@task
def knative_native_python_local(ctx, host=False):
    if host:
        working_dir = FUNC_DIR
        env = copy(os.environ)
        env.update({
            "LOG_LEVEL": "debug",
            "FAASM_INVOKE_HOST": "0.0.0.0",
            "FAASM_INVOKE_PORT": "8080",
            "HOST_TYPE": "knative",
        })

        call("./run_knative_native.sh", cwd=working_dir, env=env, shell=True)
    else:
        img_name = "faasm/knative-native-python"
        _do_knative_native_local(img_name)


def _do_knative_native_local(img_name):
    version = get_faasm_version()
    img_name = "{}:{}".format(img_name, version)

    cmd = [
        "docker", "run",
        "-p 8080:8080",
        "--env LOG_LEVEL=debug",
        "--env FAASM_INVOKE_HOST=0.0.0.0",
        "--env FAASM_INVOKE_PORT=8080",
        "--env HOST_TYPE=knative",
        img_name
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, shell=True, cwd=PROJ_ROOT)
