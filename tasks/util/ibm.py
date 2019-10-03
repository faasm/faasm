from os.path import join, exists

from tasks.util.config import get_faasm_config
from tasks.util.env import HOME_DIR

WSKPROPS = join(HOME_DIR, ".wskprops")

IBM_PYWREN_BUCKET = "faasm-pywren"
IBM_STORAGE_ENDPOINT = "https://s3.eu-gb.cloud-object-storage.appdomain.cloud"
IBM_FUNCTIONS_ENDPOINT = "https://eu-gb.functions.cloud.ibm.com"
IBM_WORKER_NAME = "faasm-worker"
IBM_WORKER_IMAGE = "faasm/ibm-worker:latest"
IBM_FUNCTIONS_NAMESPACE = "faasm-namespace"


def get_ibm_pywren_config():
    if not exists(WSKPROPS):
        raise RuntimeError("Expected to find {}".format(WSKPROPS))

    faasm_cfg = get_faasm_config()
    api_key = faasm_cfg["IBM"]["api_key"]
    namespace_id = faasm_cfg["IBM"]["namespace_id"]

    if not api_key or not namespace_id:
        raise RuntimeError("Did not find IBM config in file")

    config = {
        "pywren": {
            "storage_bucket": IBM_PYWREN_BUCKET
        },

        "ibm": {
            "iam_api_key": api_key,
        },
        "ibm_cf": {
            "endpoint": IBM_FUNCTIONS_ENDPOINT,
            "namespace": IBM_FUNCTIONS_NAMESPACE,
            "namespace_id": namespace_id,
        },

        "ibm_cos": {
            "endpoint": IBM_STORAGE_ENDPOINT,
        }
    }

    return config
