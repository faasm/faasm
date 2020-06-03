from os.path import join
from subprocess import check_output

from faasmcli.util.env import FAASM_LOCAL_DIR, FAASM_TOOLCHAIN_FILE
from faasmcli.util.version import get_faasm_version

TOOLCHAIN_INSTALL = join(FAASM_LOCAL_DIR, "toolchain")


def _get_artifact_url(name, version=None):
    version = version if version else get_faasm_version()
    url = "https://github.com/lsds/faasm/releases/download/v{}/{}".format(version, name)
    return url


def get_sysroot_tar_name(version=None):
    version = version if version else get_faasm_version()
    return "faasm-sysroot-{}.tar.gz".format(version)


def get_sysroot_tar_path():
    tar_name = get_sysroot_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def get_sysroot_url(version=None):
    tar_name = get_sysroot_tar_name(version=version)
    return _get_artifact_url(tar_name, version=version)


def get_toolchain_tar_name(version=None):
    version = version if version else get_faasm_version()
    return "faasm-toolchain-{}.tar.gz".format(version)


def get_toolchain_tar_path(version=None):
    tar_name = get_toolchain_tar_name(version=version)
    return join(FAASM_LOCAL_DIR, tar_name)


def get_toolchain_url(version=None):
    tar_name = get_toolchain_tar_name(version=version)
    return _get_artifact_url(tar_name, version=version)


def get_runtime_tar_name(version=None):
    version = version if version else get_faasm_version()
    return "faasm-runtime-root-{}.tar.gz".format(version)


def get_runtime_tar_path():
    tar_name = get_runtime_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def get_runtime_url(version=None):
    tar_name = get_runtime_tar_name(version=version)
    return _get_artifact_url(tar_name, version=version)


def tar_toolchain():
    tar_name = get_toolchain_tar_name()
    tar_path = get_toolchain_tar_path()

    print("Creating archive of Faasm toolchain")
    check_output("tar -czf {} toolchain".format(tar_name), shell=True, cwd=FAASM_LOCAL_DIR)

    return tar_name, tar_path


def tar_runtime_root():
    tar_name = get_runtime_tar_name()
    tar_path = get_runtime_tar_path()

    # Compress
    print("Creating archive of Faasm runtime root")
    check_output("tar -czf {} runtime_root".format(tar_path), shell=True, cwd=FAASM_LOCAL_DIR)

    return tar_name, tar_path


def tar_sysroot():
    tar_name = get_sysroot_tar_name()
    tar_path = get_sysroot_tar_path()

    print("Creating archive of Faasm sysroot")
    check_output("tar -czf {} llvm-sysroot".format(tar_name), shell=True, cwd=FAASM_LOCAL_DIR)

    return tar_name, tar_path
