from os.path import join
from subprocess import call

from tasks.util.env import HOME_DIR

FAASM_HOSTFILE = join(HOME_DIR, "mpi_hostfile")
OMPI_INSTALL = "/usr/local/faasm/openmpi"

MPI_RUN = join(OMPI_INSTALL, "bin", "mpirun")


def mpi_run(executable, interface=None, hostfile=FAASM_HOSTFILE, cmdline=None):
    mpi_cmd = [
        MPI_RUN,
    ]

    if hostfile:
        mpi_cmd.append("-hostfile {}".format(hostfile))

    if interface:
        mpi_cmd.append("-mca btl_tcp_if_include {}".format(interface))

    mpi_cmd.append(executable)
    
    if cmdline:
        mpi_cmd.append(cmdline)

    mpi_cmd = " ".join(mpi_cmd)
    print(mpi_cmd)
    res = call(mpi_cmd, shell=True)

    if res != 0:
        print("MPI run failed")
        exit(1)
