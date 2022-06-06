from time import sleep
from os import makedirs, listdir
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
from faasmcli.util.k8s import template_k8s_file

LOCALHOST_IP = "127.0.0.1"

K8S_COMMON_DIR = join(PROJ_ROOT, "deploy", "k8s-common")
K8S_DIR = join(PROJ_ROOT, "deploy", "k8s")
NAMESPACE_FILE = join(K8S_COMMON_DIR, "namespace.yml")

# SGX vars
K8S_SGX_DIR = join(PROJ_ROOT, "deploy", "k8s-sgx")
K8S_SGX_TEMPLATED_DIR = join(PROJ_ROOT, "deploy", "k8s-sgx", "templated")

KNATIVE_VERSION = "1.1.0"

# Notes on Knative client
# https://github.com/knative/client/blob/main/docs/cmd/kn_service_create.md
#
# Configuring the scheduler:
# https://knative.dev/docs/serving/configuring-the-autoscaler/


def _capture_cmd_output(cmd):
    cmd = " ".join(cmd)
    print(cmd)
    res = run(cmd, shell=True, check=True, stdout=PIPE, stderr=PIPE)
    output = res.stdout.decode("utf-8")
    return output


def _get_faasm_worker_pods(label):
    # To get pods with STATUS "Running" (not "Terminating") we need to use this
    # long template. There's a long-going open discussion on GH about it:
    # https://github.com/kubernetes/kubectl/issues/450
    cmd = [
        "kubectl",
        "-n faasm",
        "get pods",
        "-l {}".format(label),
        """--template "{{range .items}}{{ if not .metadata.deletionTimestamp"""
        """ }}{{.metadata.name}}{{'\\n'}}{{end}}{{end}}" """,
    ]
    output = _capture_cmd_output(cmd)
    names = [
        "faasm" + o.strip("10") for o in output.split("faasm") if o.strip()
    ]

    cmd = [
        "kubectl",
        "-n faasm",
        "get pods",
        "-l {}".format(label),
        """--template "{{range .items}}{{ if not .metadata.deletionTimestamp"""
        """ }}{{.status.podIP}}:{{end}}{{end}}" """,
    ]
    output = _capture_cmd_output(cmd)
    ips = [o.strip() for o in output.split(":") if o.strip()]

    print("Using faasm worker pods: {}".format(ips))
    return names, ips


@task
def delete_worker(ctx, sgx=False):
    """
    Delete the Faasm worker pod
    """
    # Flush redis
    cmd = "kubectl exec -n faasm redis-queue -- redis-cli flushall"
    run(cmd, shell=True, check=True)

    if sgx:
        # Delete namespace and then workers
        cmd = "kubectl delete service worker-lb -n faasm"
        print(cmd)
        run(cmd, shell=True, check=True)

        cmd = "kubectl delete --all -f {}".format(K8S_SGX_TEMPLATED_DIR)
        print(cmd)
        run(cmd, shell=True, check=True)
    else:
        cmd = "kn -n faasm service delete faasm-worker"
        print(cmd)
        run(cmd, shell=True, check=True)


@task
def deploy(ctx, replicas=0, sgx=False):
    """
    Deploy Faasm to knative
    """
    _deploy_faasm_services(sgx)

    replicas = int(replicas)
    print("Deploying Faasm worker with {} fixed replicas".format(replicas))
    if sgx:
        _deploy_faasm_worker_sgx(replicas)
    else:
        _deploy_faasm_worker(replicas)

    ini_file(ctx, sgx=sgx)


def wait_for_faasm_pods(label):
    # Wait for the faasm pods to be ready
    while True:
        print("Waiting for Faasm pods...")
        cmd = [
            "kubectl",
            "-n faasm",
            "get pods -l {}".format(label),
            "-o jsonpath='{..status.conditions[?(@.type==\"Ready\")].status}'",
        ]

        output = _capture_cmd_output(cmd)
        statuses = [o.strip() for o in output.split(" ") if o.strip()]
        if all([s == "True" for s in statuses]):
            print("All Faasm pods ready, continuing...")
            break

        print("Faasm pods not ready, waiting ({})".format(output))
        sleep(5)


def _deploy_faasm_services(sgx=False):
    # Set up the namespace first, then the rest, excluding the worker
    run(
        "kubectl apply -f {}".format(NAMESPACE_FILE),
        check=True,
        shell=True,
    )

    # Apply all files in the common directory, and the corresponding upload
    # service
    k8s_files = listdir(K8S_COMMON_DIR)
    k8s_files = [
        join(K8S_COMMON_DIR, f) for f in k8s_files if f.endswith(".yml")
    ]
    k8s_files.append(join(K8S_SGX_DIR if sgx else K8S_DIR, "upload.yml"))

    print("Applying k8s files: {}".format(k8s_files))

    for file_path in k8s_files:
        run(
            "kubectl apply -f {}".format(file_path),
            check=True,
            shell=True,
        )

    wait_for_faasm_pods("app=faasm")


def _deploy_faasm_worker(replicas):
    cmd = [
        "kn",
        "service",
        "-n faasm",
        "apply",
        "-f",
        join(K8S_SGX_DIR if sgx else K8S_DIR, "worker.yml"),
        "--wait",
    ]

    # Fix number of replicas if provided
    if replicas > 0:
        cmd.append("--scale-min={}".format(replicas))
        cmd.append("--scale-max={}".format(replicas))

    cmd_string = " ".join(cmd)
    print(cmd_string)
    run(cmd_string, shell=True, check=True)


def _deploy_faasm_worker_sgx(replicas):
    # First template the deployment file with the right number of replicas
    template_file = join(K8S_SGX_DIR, "worker.yml.j2")
    output_file = join(K8S_SGX_TEMPLATED_DIR, "worker.yml")
    template_vars = {"num_faasm_workers": replicas}
    template_k8s_file(template_file, output_file, template_vars)

    # Second, apply the deployment and the load balancer
    k8s_worker_sgx_files = [output_file, join(K8S_SGX_DIR, "worker-lb.yml")]

    print("Applying k8s files: {}".format(k8s_worker_sgx_files))

    for file_path in k8s_worker_sgx_files:
        run(
            "kubectl apply -f {}".format(file_path),
            check=True,
            shell=True,
        )

    wait_for_faasm_pods("run=faasm-worker")


@task
def delete_full(ctx, local=False):
    """
    Fully delete Faasm from Knative
    """
    # First hard-delete the worker
    delete_worker(ctx)

    # Delete the rest
    for dir_to_delete in [K8S_DIR, K8S_SGX_DIR, K8S_COMMON_DIR]:
        cmd = "kubectl delete --all -f {}".format(dir_to_delete)
        print(cmd)
        run(cmd, shell=True, check=True)


def _kn_github_url(repo, filename):
    url = [
        "https://github.com/",
        repo,
        "/releases/download/knative-v{}/".format(KNATIVE_VERSION),
        filename,
    ]
    return "".join(url)


@task
def install(ctx, reverse=False):
    """
    Install knative on an existing k8s cluster
    """

    # See docs:
    # https://knative.dev/docs/admin/install/serving/install-serving-with-yaml/

    action = "delete" if reverse else "apply"

    for s in ["serving-crds.yaml", "serving-core.yaml"]:
        url = _kn_github_url("knative/serving", s)
        run("kubectl {} -f {}".format(action, url), shell=True, check=True)

    # If installing Istio, apply CRDs first
    istio_url = _kn_github_url("knative/net-istio", "istio.yaml")
    if not reverse:
        run(
            "kubectl apply -l knative.dev/crd-install=true -f {}".format(
                istio_url
            ),
            shell=True,
            check=True,
        )

    # Run the Istio apply/ delete
    run("kubectl {} -f {}".format(action, istio_url), shell=True, check=True)

    # Set up knative networking
    net_url = _kn_github_url("knative/net-istio", "net-istio.yaml")
    run("kubectl {} -f {}".format(action, net_url), shell=True, check=True)


@task
def ini_file(ctx, local=False, publicip=None, sgx=False):
    """
    Set up the faasm config file for interacting with k8s
    """
    makedirs(GLOBAL_FAASM_CONFIG_DIR, exist_ok=True)

    if local:
        print("\n----- Setting up local config -----\n")
        knative_host = LOCALHOST_IP
        invoke_port = "8080"
        invoke_ip = LOCALHOST_IP
        upload_ip = LOCALHOST_IP
        upload_port = "8002"

        knative_host = "foo"

        worker_names = list()
        worker_ips = list()
    else:
        if sgx:
            knative_host = "foo"
        else:
            knative_host = _capture_cmd_output(
                [
                    "kn",
                    "-n faasm",
                    "service",
                    "describe",
                    "faasm-worker",
                    "-o url",
                ]
            )
            knative_host = knative_host.strip()

            # We have to remove the http:// prefix from this url otherwise Istio
            # won't recognise it
            knative_host = knative_host.replace("http://", "")

        # If we're running on bare metal or VMs, we need to use the IP of the
        # host provided for both invocations and upload, but instead of using a
        # different load balancer for each, we must use the underlying node port
        if publicip:
            print("\n----- Setting up bare metal k8s config -----\n")

            invoke_ip = publicip
            upload_ip = publicip

            invoke_port = _capture_cmd_output(
                [
                    "kubectl",
                    "-n istio-system",
                    "get",
                    "service",
                    "istio-ingressgateway",
                    "-o 'jsonpath={.spec.ports[?(@.name==\"http2\")].nodePort}'",
                ]
            )

            upload_port = _capture_cmd_output(
                [
                    "kubectl",
                    "-n faasm",
                    "get",
                    "service",
                    "upload-lb",
                    "-o 'jsonpath={.spec.ports[0].nodePort}'",
                ]
            )
        else:
            print("\n----- Extracting info from k8s -----\n")
            # If running an SGX-enabled cluster, we deploy the workers manually
            # and not through Knative
            if sgx:
                invoke_ip = _capture_cmd_output(
                    [
                        "kubectl",
                        "-n faasm",
                        "get",
                        "service",
                        "worker-lb",
                        "-o 'jsonpath={.status.loadBalancer.ingress[0].ip}'",
                    ]
                )

                invoke_port = _capture_cmd_output(
                    [
                        "kubectl",
                        "-n faasm",
                        "get",
                        "service",
                        "worker-lb",
                        "-o 'jsonpath={.spec.ports[0].port}'",
                    ]
                )
            else:
                invoke_ip = _capture_cmd_output(
                    [
                        "kubectl",
                        "-n istio-system",
                        "get",
                        "service",
                        "istio-ingressgateway",
                        "-o 'jsonpath={.status.loadBalancer.ingress[0].ip}'",
                    ]
                )
                invoke_port = 80

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

        # Get worker pods with the right label
        worker_names, worker_ips = _get_faasm_worker_pods(
            "run=faasm-worker"
            if sgx
            else "serving.knative.dev/service=faasm-worker"
        )

    print("\n----- INI file -----\n")
    print("Overwriting config file at {}\n".format(FAASM_CONFIG_FILE))

    with open(FAASM_CONFIG_FILE, "w") as fh:
        fh.write("[Faasm]\n")

        # This comment line can't be outside of the Faasm section
        fh.write("# Auto-generated at {}\n".format(datetime.now()))

        fh.write("invoke_host = {}\n".format(invoke_ip))
        fh.write("invoke_port = {}\n".format(invoke_port))
        fh.write("upload_host = {}\n".format(upload_ip))
        fh.write("upload_port = {}\n".format(upload_port))
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
