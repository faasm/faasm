from os.path import join
from os.path import join
from subprocess import check_output

from tasks.util.env import FAASM_LOCAL_DIR
from tasks.util.version import get_faasm_version

TOOLCHAIN_INSTALL = join(FAASM_LOCAL_DIR, "toolchain")


def get_sysroot_tar_name(version=None):
    version = version if version else get_faasm_version()
    return "faasm-sysroot-{}.tar.gz".format(version)


def get_sysroot_tar_path():
    tar_name = get_sysroot_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def get_toolchain_tar_name(version=None):
    version = version if version else get_faasm_version()
    return "faasm-toolchain-{}.tar.gz".format(version)


def get_toolchain_tar_path():
    tar_name = get_toolchain_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def get_runtime_tar_name(version=None):
    version = version if version else get_faasm_version()
    return "faasm-runtime-root-{}.tar.gz".format(version)


def get_runtime_tar_path():
    tar_name = get_runtime_tar_name()
    return join(FAASM_LOCAL_DIR, tar_name)


def tar_toolchain():
    tar_name = get_toolchain_tar_name()
    tar_path = get_toolchain_tar_path()

    print("Creating archive of Faasm toolchain")
    check_output("tar -cf {} toolchain".format(tar_name), shell=True, cwd=FAASM_LOCAL_DIR)

    return tar_name, tar_path


def tar_runtime_root():
    tar_name = get_runtime_tar_name()
    tar_path = get_runtime_tar_path()

    # Compress
    print("Creating archive of Faasm runtime root")
    check_output("tar -cf {} runtime_root".format(tar_path), shell=True, cwd=FAASM_LOCAL_DIR)


def tar_sysroot():
    tar_name = get_sysroot_tar_name()
    tar_path = get_sysroot_tar_path()

    print("Creating archive of Faasm sysroot")
    check_output("tar -cf {} llvm-sysroot".format(tar_name), shell=True, cwd=FAASM_LOCAL_DIR)

    return tar_name, tar_path
