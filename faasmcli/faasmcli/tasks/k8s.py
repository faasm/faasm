from datetime import datetime
from os import makedirs, listdir
from os.path import basename, join
from subprocess import run, PIPE
from time import sleep

from invoke import task

from faasmcli.util.env import (
    PROJ_ROOT,
    FAASM_CONFIG_FILE,
    GLOBAL_FAASM_CONFIG_FILE,
    GLOBAL_FAASM_CONFIG_DIR,
)
from faasmcli.util.k8s import template_k8s_file
from faasmcli.util.version import get_faasm_version

LOCALHOST_IP = "127.0.0.1"

K8S_DIR = join(PROJ_ROOT, "deploy", "k8s")
K8S_TEMPLATED_DIR = join(PROJ_ROOT, "deploy", "k8s", "templated")
NAMESPACE_FILE = join(K8S_DIR, "namespace.yml")


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
def deploy(ctx, workers, sgx=False):
    """
    Deploy Faasm to a k8s cluster
    """
    _deploy_faasm_services(int(workers), sgx)

    ini_file(ctx)


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


def wait_for_faasm_lb(service_name):
    """
    Wait for load balancers to be assigned an ingress IP
    """
    while True:
        print("Waiting for ingress IPs to be assigned")
        cmd = [
            "kubectl",
            "-n faasm",
            "get service {}".format(service_name),
            "-o jsonpath='{.status.loadBalancer.ingress[0].ip}'"
        ]

        output = _capture_cmd_output(cmd)
        if output != "":
            print("Load balancer ({}) ready".format(service_name))
            break

        print("Load balancer ({}) not ready".format(service_name))
        sleep(5)


def _deploy_faasm_services(worker_replicas, sgx=False):
    # Set up the namespace first
    run(
        "kubectl apply -f {}".format(NAMESPACE_FILE),
        check=True,
        shell=True,
    )

    # List files that need to be templated
    templated_k8s_files = listdir(K8S_DIR)
    templated_k8s_files = [
        join(K8S_DIR, f) for f in templated_k8s_files if f.endswith(".yml.j2")
    ]

    # Template variables
    template_vars = {
        "faasm_version": get_faasm_version(),
        "wasm_vm": "sgx" if sgx else "wavm",
        "python_codegen": "off" if sgx else "on",
        "num_faasm_workers": worker_replicas,
        "worker_container_name": "worker-sgx" if sgx else "worker",
    }
    if sgx:
        template_vars.update({
            "resources": {
                "resources": {
                    "limits": {"sgx.intel.com/epc": "10Mi"},
                    "requests": {"sgx.intel.com/epc": "10Mi"},
                }
            },
            "volume_mounts": {
                "volumeMounts": [
                    {"name": "var-run-aesmd", "mountPath": "/var/run/aesmd"}
                ]
            },
            "extra_env_vars": [
                {"name": "SGX_AESM_ADDR", "value": "1"},
                {
                    "name": "AZ_ATTESTATION_PROVIDER_URL",
                    "value": "https://faasmattprov.eus2.attest.azure.net",
                },
            ],
            }
        )

    # Add all files that don't need templating
    k8s_files = [join(K8S_DIR, fn) for fn in listdir(K8S_DIR) if fn.endswith(".yml")]
    # Then, template all other files
    for template_file in templated_k8s_files:
        output_file = join(K8S_TEMPLATED_DIR, basename(template_file)[:-3])
        template_k8s_file(template_file, output_file, template_vars)
        k8s_files.append(output_file)

    print("Applying k8s files: {}".format(k8s_files))

    for file_path in k8s_files:
        run(
            "kubectl apply -f {}".format(file_path),
            check=True,
            shell=True,
        )

    # Wait for faasm services
    wait_for_faasm_pods("app=faasm")

    # Wait for the worker deployment
    wait_for_faasm_pods("run=faasm-worker")

    # Lastly, wait for the load balancers to be assigned ingress IPs
    wait_for_faasm_lb("worker-lb")
    wait_for_faasm_lb("upload-lb")


@task
def delete(ctx, local=False, sgx=False):
    """
    Remove Faasm's k8s cluster
    """
    # Delete the rest
    for dir_to_delete in [K8S_TEMPLATED_DIR, K8S_DIR]:
        cmd = "kubectl delete --all -f {}".format(dir_to_delete)
        print(cmd)
        run(cmd, shell=True, check=True)


@task
def ini_file(ctx, local=False, publicip=None):
    """
    Set up the faasm config file for interacting with k8s
    """
    makedirs(GLOBAL_FAASM_CONFIG_DIR, exist_ok=True)

    if local:
        print("\n----- Setting up local config -----\n")
        invoke_port = "8080"
        invoke_ip = LOCALHOST_IP
        upload_ip = LOCALHOST_IP
        upload_port = "8002"

        worker_names = list()
        worker_ips = list()
    else:
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
        worker_names, worker_ips = _get_faasm_worker_pods("run=faasm-worker")

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
