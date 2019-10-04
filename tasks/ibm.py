from os.path import join
from subprocess import check_output, call

from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.env import RUNTIME_S3_BUCKET, STATE_S3_BUCKET, MISC_S3_BUCKET, DATA_S3_BUCKET, TEST_S3_BUCKET, \
    ANSIBLE_ROOT
from tasks.util.ibm import IBM_PYWREN_BUCKET


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
        TEST_S3_BUCKET,
    ]

    for bucket_name in bucket_names:
        cmd = ["ibmcloud", "cos", "create-bucket", "--bucket", bucket_name]
        call(" ".join(cmd), shell=True)


@task
def ibm_deploy_worker(ctx, update=False):
    config = get_faasm_config()
    redis_host = config["IBM"]["redis_host"]
    api_key = config["IBM"]["api_key"]

    cmd = [
        "ibmcloud",
        "fn",
        "action",
        "update" if update else "create",
        "--param FUNCTION_STORAGE ibm",
        "--param REDIS_QUEUE_HOST {}".format(redis_host),
        "--param REDIS_QUEUE_HOST {}".format(redis_host),
        "--param IBM_API_KEY {}".format(api_key),
        "--param CGROUP_MODE off",
        "--param NETNS_MODE off",
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


@task
def ibm_set_up_redis(ctx):
    cmd = [
        "ansible-playbook",
        "-i", join(ANSIBLE_ROOT, "inventory/ibm_cloud.yml"),
        "redis_vm.yml",
    ]

    call(" ".join(cmd), cwd=ANSIBLE_ROOT, shell=True)
