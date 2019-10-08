from os.path import join
from subprocess import call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import PROJ_ROOT
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
def k8s_deploy(ctx, local=False, bare_metal=False, ibm=False):
    if not local and not bare_metal and not ibm:
        print("Must provide one flag from local, bare_metal and ibm")
        return

    faasm_conf = get_faasm_config()

    shell_env = {}

    worker_annotations = [
        "autoscaling.knative.dev/class=kpa.autoscaling.knative.dev",
        "autoscaling.knative.dev/metric=concurrency",
        "autoscaling.knative.dev/minScale=1",  # Always have one pod running
        "autoscaling.knative.dev/maxScale=10",  # Max number of pods
        "autoscaling.knative.dev/target=4",  # In-flight requests per pod
    ]

    worker_image = "faasm/knative-worker:latest"

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
