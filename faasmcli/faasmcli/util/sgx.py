from faasmcli.util.env import PROJ_ROOT
from os import environ
from os.path import exists
from subprocess import run


def check_sgx_driver():
    if not exists("/dev/sgx"):
        raise RuntimeError("SGX driver not found!")


def start_sgx_aesmd_socket():
    work_env = environ.copy()
    check_sgx_driver()

    docker_cmd = [
        "docker volume create",
        "--driver local",
        "--opt type=tmpfs",
        "--opt device=tmpfs",
        "--opt o=rw",
        "aesmd-socket"
    ]
    docker_cmd = " ".join(docker_cmd)
    run(docker_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=work_env)

    docker_cmd = "docker compose up --no-recreate -d aesmd"
    run(docker_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=work_env)
