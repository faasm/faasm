from tasks.util.config import get_faasm_config


def get_kubernetes_host_port():
    faasm_config = get_faasm_config()
    if faasm_config.has_section("Kubernetes"):
        host = faasm_config["Kubernetes"].get("invoke_host", "localhost")
        port = faasm_config["Kubernetes"].get("invoke_port", 8080)

        return host, port
    else:
        return None, None


def get_kubernetes_upload_host(knative, host):
    faasm_conf = get_faasm_config()
    if knative:
        if not faasm_conf.has_section("Kubernetes"):
            host = host if host else "localhost"
        else:
            host = faasm_conf["Kubernetes"]["upload_host"]

    return host
