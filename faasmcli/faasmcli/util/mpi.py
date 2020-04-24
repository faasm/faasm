from os.path import join, exists
from subprocess import check_output

from faasmcli.util.env import HOME_DIR

FAASM_HOSTFILE = join(HOME_DIR, "mpi_hostfile")
OMPI_INSTALL = "/usr/local/faasm/openmpi"

MPI_RUN = join(OMPI_INSTALL, "bin", "mpirun")


def mpi_run(executable, iface=None, hostfile=FAASM_HOSTFILE, cmdline=None, np=None):
    mpi_cmd = [
        MPI_RUN,
    ]

    if hostfile and exists(hostfile):
        mpi_cmd.append("-hostfile {}".format(hostfile))

    if iface:
        mpi_cmd.append("-mca btl_tcp_if_include {}".format(iface))

    if np:
        mpi_cmd.append("-np {}".format(np))

    mpi_cmd.append(executable)

    if cmdline:
        mpi_cmd.append(cmdline)

    mpi_cmd = " ".join(mpi_cmd)
    print(mpi_cmd)
    output = check_output(mpi_cmd, shell=True)

    return output
