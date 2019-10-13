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


def _kubectl_apply(path, env=None):
    cmd = [
        "kubectl", "apply", "-f", path
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)

    ret_code = call(cmd_str, shell=True, env=env)

    if ret_code != 0:
        raise RuntimeError("Command failed: {}".format(cmd_str))


@task
def k8s_delete_worker(ctx):
    cmd = [
        "kn", "service", "delete", "faasm-worker", "--namespace=faasm"
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)
    call(cmd_str, shell=True)


@task
def k8s_deploy(ctx, local=False, bare_metal=False, ibm=False):
    if not local and not bare_metal and not ibm:
        print("Must provide one flag from local, bare-metal and ibm")
        return

    faasm_conf = get_faasm_config()

    shell_env = os.environ.copy()

    worker_annotations = [
        "autoscaling.knative.dev/class=kpa.autoscaling.knative.dev",
        "autoscaling.knative.dev/metric=concurrency",
        "autoscaling.knative.dev/minScale=1",  # Always have one pod running
        "autoscaling.knative.dev/maxScale=10",  # Max number of pods
        "autoscaling.knative.dev/target=4",  # In-flight requests per pod
    ]

    worker_image = "faasm/knative-worker"

    worker_env = {
        "REDIS_STATE_HOST": "redis-state",
        "REDIS_QUEUE_HOST": "redis-queue",
        "FUNCTION_STORAGE": "fileserver",
        "LOG_LEVEL": "debug",
        "CGROUP_MODE": "off",
        "NETNS_MODE": "off",
        "THREADS_PER_WORKER": "10",
        "MAX_QUEUE_RATIO": "1",
        "MAX_WORKERS_PER_FUNCTION": "10",
        "FS_MODE": "on",
    }

    if ibm:
        # IBM requires specifying custom kubeconfig
        shell_env = {
            "KUBECONFIG": get_ibm_kubeconfig(),
        }

        worker_env.update({
            "FUNCTION_STORAGE": "ibm",
            "IBM_API_KEY": faasm_conf["IBM"]["api_key"],
        })
    elif bare_metal:
        worker_env.update({
            "FUNCTION_STORAGE": "fileserver",
            "FILESERVER_URL": "http://upload:8002"
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

    # Deploy the worker itself
    cmd = [
        "kn", "service", "create", "faasm-worker",
        "--image", worker_image,
        "--namespace", "faasm",
        "--force",
    ]

    # Add annotations
    for annotation in worker_annotations:
        cmd.append("--annotation {}".format(annotation))

    # Add environment
    for key, value in worker_env.items():
        cmd.append("--env {}={}".format(key, value))

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, shell=True, env=shell_env)


@task
def build_knative_native(ctx, user, function, host=False, clean=False):
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
        tag_name = "faasm/{}-knative".format(function)
        cmd = [
            "docker",
            "build",
            "-t", tag_name,
            "--build-arg", "user={}".format(user),
            "--build-arg", "func={}".format(function),
            "-f", "docker/knative-native.dockerfile",
            "."
        ]

        cmd_string = " ".join(cmd)
        print(cmd_string)
        call(cmd_string, shell=True, cwd=PROJ_ROOT)
