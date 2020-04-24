from faasmcli.util.config import get_faasm_config


def get_upload_host_port(host_in, port_in):
    faasm_config = get_faasm_config()

    if not host_in and faasm_config.has_section("Faasm"):
        host = faasm_config["Faasm"].get("upload_host", "127.0.0.1")
        port = faasm_config["Faasm"].get("upload_port", 8002)
    else:
        host = host_in if host_in else "127.0.0.1"
        port = port_in if port_in else 8002

    return host, port


def get_invoke_host_port(host_in=None, port_in=None):
    faasm_config = get_faasm_config()

    if not host_in and faasm_config.has_section("Faasm"):
        host = faasm_config["Faasm"].get("invoke_host", "localhost")
        port = faasm_config["Faasm"].get("invoke_port", 8080)
    else:
        host = host_in if host_in else "127.0.0.1"
        port = port_in if port_in else 8080

    return host, port


def get_kubernetes_upload_host(knative, host):
    faasm_conf = get_faasm_config()
    if knative:
        if not faasm_conf.has_section("Faasm"):
            host = host if host else "localhost"
        else:
            host = faasm_conf["Faasm"]["upload_host"]

    return host


def is_kubernetes():
    faasm_conf = get_faasm_config()
    return faasm_conf.has_section("Faasm")
