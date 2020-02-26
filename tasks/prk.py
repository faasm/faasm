from os.path import join

from invoke import task

from tasks.util.endpoints import get_invoke_host_port
from tasks.util.env import FAASM_HOME
from tasks.util.invoke import invoke_impl
from tasks.util.mpi import mpi_run

ITERATIONS = 20
FAASM_USER = "prk"

# See the scripts in third-party/ParResKernels/scripts for the arguments they
# recommend for running the kernels.
PRK_CMDLINE = {
    "nstream": "{} 10000000 0".format(ITERATIONS),  # iterations, vector length, offset
}

PRK_NATIVE_BUILD = join(FAASM_HOME, "ParResKernels")
PRK_NATIVE_EXECUTABLES = {
    "nstream": join(PRK_NATIVE_BUILD, "MPI1", "Nstream", "nstream")
}

PRK_STATS = {
    "nstream": ("Avg time (s)", "Rate (MB/s)"),
}


@task
def invoke_prk(ctx, func, native=False, iface=None):
    if func not in PRK_CMDLINE:
        print("Invalid PRK function {}".format(func))
        return 1

    cmdline = PRK_CMDLINE[func]

    if native:
        executable = PRK_NATIVE_EXECUTABLES[func]
        cmd_out = mpi_run(executable, iface=iface, cmdline=cmdline)
        cmd_out = cmd_out.decode()
        print(cmd_out)
    else:
        host, port = get_invoke_host_port()
        cmd_out = invoke_impl(FAASM_USER, func, cmdline=cmdline, host=host, port=port)

    _parse_prk_out(func, cmd_out)


def _parse_prk_out(func, cmd_out):
    stats = PRK_STATS.get(func)

    if not stats:
        print("No stats for {}".format(func))
        return

    print("----- {} stats -----".format(func))

    for stat in stats:
        spilt_str = "{}: ".format(stat)
        stat_val = cmd_out.split(spilt_str)[1]
        stat_val = stat_val.split(" ")[0]
        stat_val = float(stat_val)

        print("{} = {}".format(stat, stat_val))
