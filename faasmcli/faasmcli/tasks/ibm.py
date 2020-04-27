import pprint
from os.path import join
from subprocess import check_output, call

import requests
from invoke import task

from faasmcli.util.config import get_faasm_config
from faasmcli.util.env import RUNTIME_S3_BUCKET, DATA_S3_BUCKET, TEST_S3_BUCKET, ANSIBLE_ROOT
from faasmcli.util.ibm import get_ibm_kubeconfig


@task
def ibm_login(ctx):
    config = get_faasm_config()
    api_key = config["IBM"]["api_key"]
    email = config["IBM"]["email"]

    cmd = ["ibmcloud", "login", "--apikey", api_key]
    call(" ".join(cmd), shell=True)

    cmd = ["ibmcloud", "target", "-o", email, "-s", "dev"]
    call(" ".join(cmd), shell=True)


@task
def ibm_ssh_redis(ctx):
    config = get_faasm_config()
    cmd = ["ssh", "root@{}".format(config["IBM"]["redis_host_public"])]
    call(" ".join(cmd), shell=True)


@task
def ibm_set_up_cos(ctx):
    cmd = ["ibmcloud", "cos", "config", "auth", "iam"]
    call(" ".join(cmd), shell=True)

    cmd = ["ibmcloud", "cos", "config", "region", "eu-gb"]
    call(" ".join(cmd), shell=True)

    call("ibmcloud cos config list", shell=True)


@task
def ibm_create_buckets(ctx):
    bucket_names = [
        RUNTIME_S3_BUCKET,
        DATA_S3_BUCKET,
        TEST_S3_BUCKET,
    ]

    for bucket_name in bucket_names:
        cmd = ["ibmcloud", "cos", "create-bucket", "--bucket", bucket_name]
        call(" ".join(cmd), shell=True)


@task
def ibm_deploy_worker(ctx, update=False):
    config = get_faasm_config()
    redis_host = config["IBM"]["redis_host_private"]
    api_key = config["IBM"]["api_key"]

    # Note that concurrency here is _intra_ container, i.e. how many concurrent
    # invocations can each container support
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "update" if update else "create",
        "--param FUNCTION_STORAGE ibm",
        "--param REDIS_QUEUE_HOST {}".format(redis_host),
        "--param REDIS_STATE_HOST {}".format(redis_host),
        "--param IBM_API_KEY {}".format(api_key),
        "--param CGROUP_MODE off",
        "--param NETNS_MODE off",
        "--memory 1024",
        "--timeout 30000",
        "--concurrency 20",
        "faasm-worker",
        "--docker",
        "faasm/ibm-worker:0.1.5"
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, shell=True)

    print("Done")


@task
def ibm_delete_worker(ctx, update=False):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "delete",
        "faasm-worker",
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, shell=True)

    print("Done")


@task
def ibm_codegen(ctx, user, func, local=False):
    if local:
        payload = {
            "value": {
                "user": user,
                "function": func,
                "mode": "codegen",
            }
        }

        response = requests.post("http://localhost:8080/run/", json=payload)
        if response.status_code != 200:
            print("Error: {}".format(response.status_code))
            pprint.pprint(response.json())
        else:
            response_data = response.json()
            print(response_data["result"])
    else:
        _do_invoke(user, func, "codegen", "")


@task
def ibm_invoke(ctx, user, func, input=None):
    _do_invoke(user, func, "invoke", input)


def _do_invoke(user, func, mode, input):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "invoke",
        "--blocking",
        "--param", "user", user,
        "--param", "function", func,
        "--param", "mode", mode,
        "--param", "input", input,
        "faasm-worker",
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    call(cmd_string, shell=True)
    print("Done")


@task
def ibm_list(ctx):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "list",
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    check_output(cmd_string, shell=True)
    print("Done")


@task
def ibm_set_up_redis(ctx):
    cmd = [
        "ansible-playbook",
        "-i", join(ANSIBLE_ROOT, "inventory/ibm_cloud.yml"),
        "redis_vm.yml",
    ]

    call(" ".join(cmd), cwd=ANSIBLE_ROOT, shell=True)


@task
def ibm_k8s_add_knative(ctx):
    faasm_conf = get_faasm_config()

    print("Enabling knative")
    cmd = [
        "ibmcloud", "ks", "cluster-addon-enable", "knative", "--cluster", faasm_conf["IBM"]["k8s_cluster_id"],
    ]

    call(" ".join(cmd), shell=True)


@task
def ibm_k8s_config(ctx):
    faasm_conf = get_faasm_config()

    print("Getting cluster config")
    cmd = [
        "ibmcloud", "ks", "cluster", "config", "--cluster", faasm_conf["IBM"]["k8s_cluster_id"]
    ]

    call(" ".join(cmd), shell=True)


@task
def ibm_print_kubeconf(ctx):
    print("export KUBECONFIG={}".format(get_ibm_kubeconfig()))
