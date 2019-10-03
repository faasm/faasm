from subprocess import check_output, call

from invoke import task
from pywren_ibm_cloud.compute.backends.ibm_cf.ibm_cf import IBMCloudFunctionsBackend

from tasks.util.config import get_faasm_config
from tasks.util.env import FAASM_RUNTIME_ROOT, RUNTIME_S3_BUCKET, STATE_S3_BUCKET, MISC_S3_BUCKET, DATA_S3_BUCKET
from tasks.util.ibm import get_ibm_pywren_config, IBM_WORKER_NAME, IBM_WORKER_IMAGE, IBM_PYWREN_BUCKET
import pywren_ibm_cloud as pywren


def add_seven(x):
    return x + 7


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
def ibm_set_up_cos(ctx):
    cmd = ["ibmcloud", "cos", "config", "auth", "iam"]
    call(" ".join(cmd), shell=True)

    cmd = ["ibmcloud", "cos", "config", "region", "eu-gb"]
    call(" ".join(cmd), shell=True)

    call("ibmcloud cos config list", shell=True)


@task
def ibm_create_buckets(ctx):
    bucket_names = [
        IBM_PYWREN_BUCKET,
        RUNTIME_S3_BUCKET,
        STATE_S3_BUCKET,
        MISC_S3_BUCKET,
        DATA_S3_BUCKET,
    ]

    for bucket_name in bucket_names:
        cmd = ["ibmcloud", "cos", "create-bucket", "--bucket", bucket_name]
        call(" ".join(cmd), shell=True)


@task
def ibm_deploy_worker(ctx, update=False):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "update" if update else "create",
        "faasm-worker",
        "--docker",
        "faasm/ibm-worker:latest"
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    check_output(cmd_string, shell=True)

    print("Done")


@task
def ibm_invoke_worker(ctx):
    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "invoke",
        "--blocking",
        "faasm-worker"
    ]

    cmd_string = " ".join(cmd)
    print(cmd_string)
    check_output(cmd_string, shell=True)
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
