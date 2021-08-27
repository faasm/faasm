from time import sleep
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

KNATIVE_VERSION = "0.25.0"

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


def _capture_cmd_output(cmd):
    cmd = " ".join(cmd)
    print(cmd)
    res = run(cmd, shell=True, check=True, stdout=PIPE, stderr=PIPE)
    output = res.stdout.decode("utf-8")
    return output


def _get_faasm_worker_pods():
    knative_service = _get_knative_service()
    cmd = [
        "kubectl",
        "-n faasm",
        "get pods",
        "-l serving.knative.dev/service={}".format(knative_service),
        "-o jsonpath='{range .items[*]}{@.metadata.name}{\" \"}{end}'",
    ]
    output = _capture_cmd_output(cmd)
    names = [o.strip() for o in output.split(" ") if o.strip()]

    cmd = [
        "kubectl",
        "-n faasm",
        "get pods",
        "-l serving.knative.dev/service={}".format(knative_service),
        "-o jsonpath='{range .items[*]}{@.status.podIP}{\" \"}{end}'",
    ]
    output = _capture_cmd_output(cmd)
    ips = [o.strip() for o in output.split(" ") if o.strip()]

    print("Using faasm worker pods: {}".format(ips))
    return names, ips


def _get_knative_service(ok_none=False):
    cmd = [
        "kn",
        "-n faasm",
        "service",
        "list",
        "-o jsonpath='{range .items[*]}{@.metadata.name}{\" \"}{end}'",
    ]
    output = _capture_cmd_output(cmd)
    names = [o.strip() for o in output.split(" ") if o.strip()]

    if ok_none and len(names) == 0:
        return None

    if len(names) != 1:
        raise RuntimeError(
            "Could not find single knative service in {}".format(output)
        )

    return names[0]


@task
def delete_worker(ctx):
    """
    Delete the Faasm worker pod
    """
    kn_service = _get_knative_service(ok_none=True)
    if not kn_service:
        print("No worker found to delete")
        return

    # Flush redis
    cmd = "kubectl exec -n faasm redis-queue -- redis-cli flushall"
    run(cmd, shell=True, check=True)

    cmd = "kn -n faasm service delete {}".format(kn_service)
    print(cmd)
    run(cmd, shell=True, check=True)


@task
def deploy(ctx, replicas=DEFAULT_REPLICAS):
    """
    Deploy Faasm to knative
    """
    _deploy_faasm_services()

    _deploy_faasm_worker(replicas)

    ini_file(ctx)


def _deploy_faasm_services():
    # Set up the namespace first, then the rest
    run(
        "kubectl apply -f {}".format(NAMESPACE_FILE),
        check=True,
        shell=True,
    )

    run(
        "kubectl apply -f {}".format(K8S_DIR),
        check=True,
        shell=True,
    )

    # Wait for the faasm pods to be ready
    while True:
        print("Waiting for Faasm pods...")
        cmd = [
            "kubectl",
            "-n faasm",
            "get pods -l app=faasm",
            "-o jsonpath='{..status.conditions[?(@.type==\"Ready\")].status}'",
        ]

        output = _capture_cmd_output(cmd)
        statuses = [o.strip() for o in output.split(" ") if o.strip()]
        if all([s == "True" for s in statuses]):
            print("All Faasm pods ready, continuing...")
            break

        print("Faasm pods not ready, waiting ({})".format(output))
        sleep(5)


def _deploy_faasm_worker(replicas):
    # Deploy the knative function
    worker_name = "faasm-worker-{}".format(
        datetime.now().strftime("%y%m%d-%H%M")
    )

    faasm_ver = get_faasm_version()
    image = "{}:{}".format(FAASM_WORKER_IMAGE, faasm_ver)

    cmd = [
        "kn",
        "service",
        "create",
        worker_name,
        "--image {}".format(image),
        "--namespace faasm",
        "--min-scale={}".format(replicas),
        "--max-scale={}".format(replicas),
    ]

    # Add annotations
    for annotation in FAASM_WORKER_ANNOTATIONS:
        cmd.append("--annotation {}".format(annotation))

    # Add standard environment
    for key, value in KNATIVE_ENV.items():
        cmd.append("--env {}={}".format(key, value))

    cmd_string = " ".join(cmd)
    print(cmd_string)

    run(cmd_string, shell=True, check=True)


@task
def delete_full(ctx, local=False):
    """
    Fully delete Faasm from Knative
    """
    # First hard-delete the worker
    delete_worker(ctx)

    # Delete the rest
    run("kubectl delete --all -f {}".format(K8S_DIR), shell=True, check=True)


@task
def install(ctx, reverse=False):
    """
    Install knative on an existing k8s cluster
    """

    # See docs:
    # https://knative.dev/docs/admin/install/serving/install-serving-with-yaml/

    def github_url(repo, filename):
        url = [
            "https://github.com/",
            repo,
            "/releases/download/v{}/".format(KNATIVE_VERSION),
            filename,
        ]
        return "".join(url)

    action = "delete" if reverse else "apply"

    for s in ["serving-crds.yaml", "serving-core.yaml"]:
        url = github_url("knative/serving", s)
        run("kubectl {} -f {}".format(action, url), shell=True, check=True)

    # If installing, apply CRDs first
    if not reverse:
        url = github_url("knative/net-istio", "istio.yaml")

        run(
            "kubectl apply -l knative.dev/crd-install=true -f {}".format(url),
            shell=True,
            check=True,
        )

    for s in ["istio.yaml", "net-istio.yaml"]:
        url = github_url("knative/net-istio", s)
        run("kubectl {} -f {}".format(action, url), shell=True, check=True)


@task
def ini_file(ctx):
    """
    Set up the faasm config file for interacting with k8s
    """
    makedirs(GLOBAL_FAASM_CONFIG_DIR, exist_ok=True)

    print("\n----- Extracting info from k8s -----\n")
    knative_service = _get_knative_service()
    knative_host = _capture_cmd_output(
        [
            "kn",
            "-n faasm",
            "service",
            "describe",
            knative_service,
            "-o url",
        ]
    )
    knative_host = knative_host.strip()

    # NOTE: we have to remove the http:// prefix from this url otherwise Istio
    # won't recognise it
    knative_host = knative_host.replace("http://", "")

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
    upload_port = _capture_cmd_output(
        [
            "kubectl",
            "-n faasm",
            "get",
            "service",
            "upload-lb",
            "-o 'jsonpath={.spec.ports[0].port}'",
        ]
    )

    worker_names, worker_ips = _get_faasm_worker_pods()

    print("\n----- INI file -----\n")
    print("Overwriting config file at {}\n".format(FAASM_CONFIG_FILE))

    with open(FAASM_CONFIG_FILE, "w") as fh:
        fh.write("[Faasm]\n")
        fh.write("# Auto-generated at {}\n".format(datetime.now()))
        fh.write("invoke_host = {}\n".format(istio_ip))
        fh.write("invoke_port = {}\n".format(istio_port))
        fh.write("upload_host = {}\n".format(upload_ip))
        fh.write("upload_port= {}\n".format(upload_port))
        fh.write("knative_host = {}\n".format(knative_host))
        fh.write("worker_names = {}\n".format(",".join(worker_names)))
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
        "curl -X PUT http://{}:{}/f/<user>/<func> -T <wasm_file>".format(
            upload_ip, upload_port
        )
    )
    print("")
