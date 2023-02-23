from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.sgx import start_sgx_aesmd_socket
from faasmcli.util.version import get_version
from invoke import task
from os import environ
from subprocess import run

ALLOWED_SERVICES = {
    "cpp": "cpp",
    "faasm": "faasm-cli",
    "faasm-sgx-sim": "faasm-cli",
    "faasm-sgx": "faasm-cli",
    "python": "python",
}


@task(default=True, optional=["detached"])
def cli(ctx, service, detached=False):
    """
    Get a shell into one of the service containers: cpp, faasm, or python
    """
    if service not in ALLOWED_SERVICES:
        print("Unrecognised service for CLI: {}".format(service))
        print("Service must be one in {}".format(ALLOWED_SERVICES.keys()))
        raise RuntimeError("Unrecognised service for CLI")

    work_env = environ.copy()
    faasm_ver = get_version()
    if service == "faasm-sgx":
        work_env["WASM_VM"] = "sgx"
        work_env["FAASM_CLI_IMAGE"] = "faasm/cli-sgx:{}".format(faasm_ver)
        work_env["FAASM_CLI_IMAGE"] = "faasm/worker-sgx:{}".format(faasm_ver)
        work_env["SGX_DEVICE_MOUNT_DIR"] = "/dev/sgx"
        work_env["AESMD_SOCKET_EXTERNAL_VOLUME"] = "true"
        start_sgx_aesmd_socket()
    elif service == "faasm-sgx-sim":
        work_env["WASM_VM"] = "sgx"
        work_env["FAASM_CLI_IMAGE"] = "faasm/cli-sgx-sim:{}".format(faasm_ver)
        work_env["FAASM_CLI_IMAGE"] = "faasm/worker-sgx-sim:{}".format(faasm_ver)

    # In GHA we want to be able to pass a string argument to detached, yet
    # disable it. So passing "False" also disables the detached flag
    if detached and detached != "False":
        work_env["FAASM_BUILD_MOUNT"] = "/host_dev/build"
        work_env["FAASM_LOCAL_MOUNT"] = "/host_dev/faasm-local"
    else:
        work_env["FAASM_BUILD_MOUNT"] = "/build/faasm"
        work_env["FAASM_LOCAL_MOUNT"] = "/usr/local/faasm"

    docker_cmd = [
        "docker compose up",
        "--no-recreate",
        "-d",
        ALLOWED_SERVICES[service],
    ]
    docker_cmd = " ".join(docker_cmd)
    run(docker_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=work_env)

    docker_cmd = [
        "docker compose exec",
        ALLOWED_SERVICES[service],
        work_env["SHELL"],
    ]
    docker_cmd = " ".join(docker_cmd)
    run(docker_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=work_env)
