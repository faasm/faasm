from os.path import join

from faasmcli.util.config import get_faasm_config
from faasmcli.util.env import HOME_DIR


def get_ibm_kubeconfig():
    # NOTE: we assume LON02 datacentre here
    faasm_config = get_faasm_config()
    return join(
        HOME_DIR,
        ".bluemix/plugins/container-service/clusters",
        faasm_config["IBM"]["k8s_cluster_id"],
        "kube-config-lon02-faasm.yml"
    )
