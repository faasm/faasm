import os
from os import makedirs
from os.path import join
from subprocess import run, PIPE
from datetime import datetime

from invoke import task

from faasmcli.util.env import (
    PROJ_ROOT,
    FAASM_CONFIG_FILE,
    GLOBAL_FAASM_CONFIG_FILE,
    GLOBAL_FAASM_CONFIG_DIR,
)
from faasmcli.util.version import get_faasm_version


K8S_DIR = join(PROJ_ROOT, "deploy", "k8s")
NAMESPACE_FILE = join(K8S_DIR, "namespace.yml")

KNATIVE_VERSION = "0.21.0"
KNATIVE_SPECS = [
    ["knative/serving", "serving-crds.yaml"],
    ["knative/serving", "serving-core.yaml"],
    ["knative/net-istio", "istio.yaml"],
    ["knative/net-istio", "net-istio.yaml"],
    ["knative/serving", "serving-default-domain.yaml"],
]


# Number of replicas in the Faasm worker pod
DEFAULT_REPLICAS = 4

# Notes on Knative client
# https://github.com/knative/client/blob/master/docs/cmd/kn_service_create.md
#
# Configuring the scheduler:
# https://knative.dev/docs/serving/configuring-the-autoscaler/

# TODO: Use the knative autoscaler rather than hard-coding a number of workers
# Longer window means fewer knative interventions
FAASM_WORKER_ANNOTATIONS = [
    "autoscaling.knative.dev/enable-scale-to-zero=false",
    "autoscaling.knative.dev/stable-window=120s",
]

FAASM_WORKER_CONCURRENCY = 0

KNATIVE_FUNC_PREFIX = "faasm-"

FAASM_WORKER_NAME = "{}worker".format(KNATIVE_FUNC_PREFIX)
FAASM_WORKER_IMAGE = "faasm/worker"

ONE_MIN = 60000
THIRTY_SECS = 30000

KNATIVE_ENV = {
    "REDIS_STATE_HOST": "redis-state",
    "REDIS_QUEUE_HOST": "redis-queue",
    "HOST_TYPE": "knative",
    "LOG_LEVEL": "info",
    "CAPTURE_STDOUT": "on",  # Needed for experiment outputs
    "CGROUP_MODE": "off",
    "NETNS_MODE": "off",
    "PYTHON_PRELOAD": "off",  # Switch on/ off preloading of Python runtime
    "PYTHON_CODEGEN": "off",  # Switch on/ off up-front codegen for Python
    "BOUND_TIMEOUT": str(
        THIRTY_SECS
    ),  # How long a bound worker sticks around for
    "UNBOUND_TIMEOUT": str(
        10 * ONE_MIN
    ),  # How long an unbound worker sticks around for
    "GLOBAL_MESSAGE_TIMEOUT": str(
        2 * ONE_MIN
    ),  # How long things wait for messages on global bus
    "ENDPOINT_INTERFACE": "eth0",  # Assuming eth0 is accessible to other hosts
}


def _fn_name(function):
    return "{}{}".format(KNATIVE_FUNC_PREFIX, function.replace("_", "-"))


def _kubectl_cmd(path, action, env=None):
    cmd = ["kubectl", action, "-f", path]

    shell_env_dict = os.environ.copy()
    if env:
        shell_env_dict.update(env)

    cmd_str = " ".join(cmd)
    print(cmd_str)

    run(cmd_str, shell=True, check=True, env=shell_env_dict)


def _kubectl_apply(path, env=None):
    _kubectl_cmd(path, "apply", env=env)


def _kubectl_delete(path, env=None):
    _kubectl_cmd(path, "delete", env=env)


@task
def delete_worker(ctx, hard=False):
    """
    Delete the Faasm worker pod
    """
    # Clear redis queue
    cmd = "kubectl exec -n faasm redis-queue -- redis-cli flushall"
    run(cmd, shell=True, check=True)

    _delete_knative_fn("worker", hard)


@task
def deploy(ctx, replicas=DEFAULT_REPLICAS):
    """
    Deploy Faasm to knative
    """

    # Set up the namespace first, then the rest
    _kubectl_apply(NAMESPACE_FILE)
    _kubectl_apply(K8S_DIR)

    _deploy_knative_fn(
        FAASM_WORKER_NAME,
        FAASM_WORKER_IMAGE,
        replicas,
        FAASM_WORKER_CONCURRENCY,
        FAASM_WORKER_ANNOTATIONS,
    )


@task
def delete_full(ctx, local=False):
    """
    Fully delete Faasm from Knative
    """
    # First hard-delete the worker
    delete_worker(ctx, hard=True)

    # Delete the rest
    _kubectl_delete(K8S_DIR)


def _delete_knative_fn(name, hard):
    func_name = _fn_name(name)
    if hard:
        cmd = ["kn", "service", "delete", func_name, "--namespace=faasm"]

        cmd_str = " ".join(cmd)
        print(cmd_str)
        run(cmd_str, shell=True, check=True)
    else:
        # Delete the pods (they'll respawn)
        label = "serving.knative.dev/service={}".format(func_name)
        cmd = "kubectl -n faasm delete pods -l {} --wait=false --now".format(
            label
        )
        print(cmd)
        run(cmd, shell=True, check=True)


def _deploy_knative_fn(
    name,
    image,
    replicas,
    concurrency,
    annotations,
    extra_env=None,
    shell_env=None,
):
    faasm_ver = get_faasm_version()
    image = "{}:{}".format(image, faasm_ver)

    cmd = [
        "kn",
        "service",
        "create",
        name,
        "--image",
        image,
        "--namespace",
        "faasm",
        "--force",
    ]

    cmd.extend(
        {
            "--min-scale={}".format(replicas),
            "--max-scale={}".format(replicas),
            "--concurrency-limit={}".format(concurrency)
            if concurrency
            else "",
        }
    )

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

    run(cmd_string, shell=True, check=True, env=shell_env_dict)


@task
def install(ctx):
    """
    Install knative on an existing k8s cluster
    """
    for s in KNATIVE_SPECS:
        _kubectl_apply(
            "https://github.com/{}/releases/download/v{}/{}".format(
                s[0], KNATIVE_VERSION, s[1]
            )
        )


@task
def uninstall(ctx):
    """
    Uninstall knative from an existing k8s cluster
    """
    # We delete in the reverse order we apply, and the last component
    # needs not to be deleted.
    for s in reversed(KNATIVE_SPECS[:-1]):
        _kubectl_delete(
            "https://github.com/{}/releases/download/v{}/{}".format(
                s[0], KNATIVE_VERSION, s[1]
            )
        )


def _capture_cmd_output(cmd):
    cmd = " ".join(cmd)
    print(cmd)
    res = run(cmd, shell=True, check=True, stdout=PIPE, stderr=PIPE)
    output = res.stdout.decode("utf-8")
    return output


def get_faasm_worker_pods():
    kubecmd = [
        "kubectl",
        "-n faasm",
        "get pods",
        "-l serving.knative.dev/service=faasm-worker",
        "-o wide",
    ]

    kubecmd = " ".join(kubecmd)
    print(kubecmd)
    res = run(
        kubecmd,
        stdout=PIPE,
        stderr=PIPE,
        shell=True,
        check=True,
    )

    cmd_out = res.stdout.decode("utf-8")

    # Split output into list of strings
    lines = cmd_out.split("\n")[1:]
    lines = [l.strip() for l in lines if l.strip()]

    pod_names = list()
    pod_ips = list()
    for line in lines:
        line_parts = line.split(" ")
        line_parts = [p.strip() for p in line_parts if p.strip()]

        pod_names.append(line_parts[0])
        pod_ips.append(line_parts[5])

    print("Using faasm worker pods: {}".format(pod_ips))
    return pod_names, pod_ips


@task
def ini_file(ctx):
    """
    Set up the faasm config file for interacting with k8s
    """
    makedirs(GLOBAL_FAASM_CONFIG_DIR, exist_ok=True)

    print("\n----- Kubectl commands -----\n")
    # TODO - work out how to get the right kn headers
    knative_host = _capture_cmd_output(
        [
            "kubectl",
            "-n faasm",
            "get",
            "service",
            "faasm-worker",
            "-o 'jsonpath={.spec.externalName}'",
        ]
    )

    istio_ip = _capture_cmd_output(
        [
            "kubectl",
            "-n istio-system",
            "get",
            "service",
            "istio-ingressgateway",
            "-o 'jsonpath={.status.loadBalancer.ingress[0].ip}'",
        ]
    )
    istio_port = 80

    upload_ip = _capture_cmd_output(
        [
            "kubectl",
            "-n faasm",
            "get",
            "service",
            "upload-lb",
            "-o 'jsonpath={.status.loadBalancer.ingress[0].ip}'",
        ]
    )
    upload_port = 8002

    hoststats_ip = _capture_cmd_output(
        [
            "kubectl",
            "-n faasm",
            "get",
            "service",
            "hoststats-proxy-lb",
            "-o 'jsonpath={.status.loadBalancer.ingress[0].ip}'",
        ]
    )
    hoststats_port = 5000

    worker_names, worker_ips = get_faasm_worker_pods()

    print("\n----- INI file -----\n")
    print("Overwriting config file at {}\n".format(FAASM_CONFIG_FILE))

    with open(FAASM_CONFIG_FILE, "w") as fh:
        fh.write("# Auto-generated at {}\n".format(datetime.now()))
        fh.write("invoke_host = {}\n".format(istio_ip))
        fh.write("invoke_port = {}\n".format(istio_port))
        fh.write("upload_host = {}\n".format(upload_ip))
        fh.write("upload_port= {}\n".format(upload_port))
        fh.write("hoststats_host = {}\n".format(hoststats_ip))
        fh.write("hoststats_port= {}\n".format(hoststats_port))
        fh.write("knative_host = {}\n".format(knative_host))
        fh.write("worker_ips = {}\n".format(",".join(worker_ips)))

    with open(FAASM_CONFIG_FILE, "r") as fh:
        print(fh.read())

    print(
        "Symlinking {} to {}".format(
            FAASM_CONFIG_FILE, GLOBAL_FAASM_CONFIG_FILE
        )
    )

    run("rm -f {}".format(GLOBAL_FAASM_CONFIG_FILE), shell=True, check=True)
    run(
        "ln -s {} {}".format(FAASM_CONFIG_FILE, GLOBAL_FAASM_CONFIG_FILE),
        shell=True,
        check=True,
    )

    print("\n----- Examples -----\n")

    print("Invoke:")
    print("curl -H 'Host: {}' http://{}".format(knative_host, istio_ip))

    print("\nUpload:")
    print(
        "curl -X PUT http://{}:${}/f/<user>/<func> -T <wasm_file>".format(
            upload_ip, upload_port
        )
    )
    print("\n")
