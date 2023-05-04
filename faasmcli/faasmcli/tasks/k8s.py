from datetime import datetime
from os import environ, listdir, makedirs
from os.path import join
from subprocess import run, PIPE
from time import sleep

from invoke import task

from faasmcli.util.env import (
    PROJ_ROOT,
    FAASM_CONFIG_FILE,
    GLOBAL_FAASM_CONFIG_FILE,
    GLOBAL_FAASM_CONFIG_DIR,
)

LOCALHOST_IP = "127.0.0.1"

K8S_COMMON_DIR = join(PROJ_ROOT, "deploy", "k8s-common")
K8S_SGX_DIR = join(PROJ_ROOT, "deploy", "k8s-sgx")
K8S_WAVM_DIR = join(PROJ_ROOT, "deploy", "k8s")
K8S_WAMR_DIR = join(PROJ_ROOT, "deploy", "k8s-wamr")
NAMESPACE_FILE = join(K8S_COMMON_DIR, "namespace.yml")


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
def deploy(ctx, workers):
    """
    Deploy Faasm to a k8s cluster
    """

    # Pick the right WASM VM
    if "WASM_VM" in environ:
        wasm_vm = environ["WASM_VM"]
    else:
        wasm_vm = "wavm"

    _deploy_faasm_services(int(workers), wasm_vm)

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
            "-o jsonpath='{.status.loadBalancer.ingress[0].ip}'",
        ]

        output = _capture_cmd_output(cmd)
        if output != "":
            print("Load balancer ({}) ready".format(service_name))
            break

        print("Load balancer ({}) not ready".format(service_name))
        sleep(5)


def _deploy_faasm_services(worker_replicas, wasm_vm):
    # Set up the namespace first
    run(
        "kubectl apply -f {}".format(NAMESPACE_FILE),
        check=True,
        shell=True,
    )

    # First, add all the common files
    k8s_files = [
        join(K8S_COMMON_DIR, f)
        for f in listdir(K8S_COMMON_DIR)
        if f.endswith(".yml")
    ]

    # Then add the deployment specific files
    if wasm_vm == "sgx":
        k8s_dir = K8S_SGX_DIR
    elif wasm_vm == "wamr":
        k8s_dir = K8S_WAMR_DIR
    elif wasm_vm == "wavm":
        k8s_dir = K8S_WAVM_DIR
    else:
        print("Unrecognised WASM VM: {}".format(wasm_vm))
        raise RuntimeError("Unrecognised WASM VM")
    k8s_files += [
        join(k8s_dir, f) for f in listdir(k8s_dir) if f.endswith(".yml")
    ]

    # Apply all the files
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

    # Scale the worker deployment to the right number of pods, and wait again
    run(
        "kubectl -n faasm scale deployment/faasm-worker --replicas={}".format(
            worker_replicas
        ),
        check=True,
        shell=True,
    )
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
    for dir_to_delete in [
        K8S_COMMON_DIR,
        K8S_WAVM_DIR,
        K8S_WAMR_DIR,
        K8S_SGX_DIR,
    ]:
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
        planner_ip = LOCALHOST_IP
        planner_port = "8081"

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

            planner_port = _capture_cmd_output(
                [
                    "kubectl",
                    "-n faasm",
                    "get",
                    "service",
                    "planner-lb",
                    "-o 'jsonpath={.spec.ports[0].nodePort}'",
                ]
            )
        else:
            print("\n----- Extracting info from k8s -----\n")
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

            planner_ip = _capture_cmd_output(
                [
                    "kubectl",
                    "-n faasm",
                    "get",
                    "service",
                    "planner-lb",
                    "-o 'jsonpath={.status.loadBalancer.ingress[0].ip}'",
                ]
            )
            planner_port = _capture_cmd_output(
                [
                    "kubectl",
                    "-n faasm",
                    "get",
                    "service",
                    "planner-lb",
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
        fh.write("planner_host = {}\n".format(planner_ip))
        fh.write("planner_port = {}\n".format(planner_port))
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
