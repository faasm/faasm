from faasmcli.util.env import PROJ_ROOT
from os.path import join


def get_version(project="faasm"):
    def read_version_from_file(filename):
        with open(filename, "r") as fh:
            version = fh.read()
            version = version.strip()
        return version

    def read_version_from_file_path(file_path, var_name):
        with open(file_path, "r") as fh:
            lines = fh.readlines()
            lines = [line.strip() for line in lines if var_name in line]
            if len(lines) != 1:
                raise RuntimeError("Inconsistent env. file state")
            return lines[0].split("=")[-1]

    if project == "faasm":
        ver_file = join(PROJ_ROOT, "VERSION")
        return read_version_from_file(ver_file)

    env_file = join(PROJ_ROOT, ".env")
    if project == "cpp":
        old_ver = read_version_from_file_path(env_file, "CPP_VERSION")
        new_ver_file = join(PROJ_ROOT, "clients", "cpp", "VERSION")
        new_ver = read_version_from_file(new_ver_file)
        return old_ver, new_ver

    if project == "python":
        old_ver = read_version_from_file_path(env_file, "PYTHON_VERSION")
        new_ver_file = join(PROJ_ROOT, "clients", "python", "VERSION")
        new_ver = read_version_from_file(new_ver_file)
        return old_ver, new_ver

    if project == "faabric":
        old_ver = read_version_from_file_path(env_file, "FAABRIC_VERSION")
        new_ver_file = join(PROJ_ROOT, "faabric", "VERSION")
        new_ver = read_version_from_file(new_ver_file)
        return old_ver, new_ver

    if project == "faasmctl":
        reqs_file = join(PROJ_ROOT, "faasmcli", "requirements.txt")
        old_ver = read_version_from_file_path(reqs_file, "faasmctl")
        return old_ver

    print("Unrecognised project name to get version from: {}".format(project))
    raise RuntimeError("Unrecognised project name")
