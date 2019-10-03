from subprocess import check_output

from invoke import task
from pywren_ibm_cloud.libs.ibm.cloudfunctions_client import CloudFunctionsClient


def _get_ibm_config(ctx):
    config = {'pywren' : {'storage_bucket' : 'BUCKET_NAME'},

              'ibm_cf':  {'endpoint': 'HOST',
                          'namespace': 'NAMESPACE',
                          'api_key': 'API_KEY'},

              'ibm_cos': {'endpoint': 'REGION_ENDPOINT',
                          'api_key': 'API_KEY'}}



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
    cf = CloudFunctionsClient()

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
