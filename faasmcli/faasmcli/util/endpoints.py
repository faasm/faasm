from faasmcli.util.config import get_faasm_config
from os import environ


def _get_config_value(env_var, key, default_value):
    """
    Get the config value, allowing an environment variable to take precedence
    """
    faasm_config = get_faasm_config()
    env_value = environ.get(env_var, None)

    if env_value:
        return env_value

    if faasm_config.has_section("Faasm"):
        conf_value = faasm_config["Faasm"].get(key, default_value)
        return conf_value

    return default_value


def get_upload_host_port():
    host = _get_config_value("UPLOAD_HOST", "upload_host", "127.0.0.1")
    port = _get_config_value("UPLOAD_PORT", "upload_port", 8002)

    return host, port


def get_invoke_host_port():
    host = _get_config_value("INVOKE_HOST", "invoke_host", "127.0.0.1")
    port = _get_config_value("INVOKE_PORT", "invoke_port", 8080)

    return host, port


def get_knative_headers():
    host = _get_config_value("KNATIVE_HOST", "knative_host", "example.com")

    return {"Host": "{}".format(host)}

