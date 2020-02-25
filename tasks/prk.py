from os.path import join

from invoke import task

from tasks import invoke
from tasks.util.env import FAASM_HOME
from tasks.util.mpi import mpi_run

ITERATIONS = 2
FAASM_USER = "prk"

# See the scripts in third-party/ParResKernels/scripts for the arguments they
# recommend for running the kernels.
PRK_CMDLINE = {
    "nstream": "{} 200 0".format(ITERATIONS),  # iterations, vector length, offset
}

PRK_NATIVE_BUILD = join(FAASM_HOME, "ParResKernels")
PRK_NATIVE_EXECUTABLES = {
    "nstream": join(PRK_NATIVE_BUILD, "MPI1", "Nstream", "nstream")
}


@task
def invoke_prk(ctx, func, native=False, interface=None):
    if func not in PRK_CMDLINE:
        print("Invalid PRK function {}".format(func))
        return 1

    cmdline = PRK_CMDLINE[func]
    
    if native:
        executable = PRK_NATIVE_EXECUTABLES[func]
        mpi_run(executable, interface=interface, cmdline=cmdline)
    else:
        invoke(ctx, FAASM_USER, func, cmdline=cmdline)

