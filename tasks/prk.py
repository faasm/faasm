from os.path import join

from invoke import task

from tasks.util.endpoints import get_invoke_host_port
from tasks.util.env import FAASM_HOME
from tasks.util.invoke import invoke_impl
from tasks.util.mpi import mpi_run

ITERATIONS = 20
FAASM_USER = "prk"

# See the scripts in third-party/ParResKernels/scripts for the arguments they
# recommend for running the kernels:
#
# $MPIRUN -np $NUMPROCS MPI1/DGEMM/dgemm               $NUMITERS -50000 32 1;    echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Nstream/nstream           $NUMITERS 2000000000L 0;  echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Random/random             $NUMITERS 32;             echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Reduce/reduce             $NUMITERS 2000000000L;    echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Sparse/sparse             $NUMITERS 13 7;           echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Stencil/stencil           $NUMITERS 50000;          echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Synch_global/global       $NUMITERS 2000000000L;    echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Synch_p2p/p2p             $NUMITERS 70000 70000;    echo $SEPLINE
# $MPIRUN -np $NUMPROCS MPI1/Transpose/transpose       $NUMITERS 50000 64;       echo $SEPLINE
#
PRK_CMDLINE = {
    "dgemm": "{} -50000 32 1".format(ITERATIONS),  # iterations, matrix order, outer block size (?)
    "nstream": "{} 10000000 0".format(ITERATIONS),  # iterations, vector length, offset
    "random": "{} 32".format(ITERATIONS),  # iterations, update ratio
    "reduce": "{} 10000000".format(ITERATIONS),  # iterations, vector length
    "sparse": "{} 13 7".format(ITERATIONS),  # iterations, log2 grid size, stencil radius
    "stencil": "{} 50000".format(ITERATIONS),  # iterations, array dimension
    "global": "{} 10000000".format(ITERATIONS),  # iterations, scramble string length
    "p2p": "{} 30000 30000".format(ITERATIONS),  # iterations, 1st array dimension, 2nd array dimension
    "transpose": "{} 25000 64".format(ITERATIONS),  # iterations, matrix order, tile size
}

PRK_NATIVE_BUILD = join(FAASM_HOME, "ParResKernels")

PRK_NATIVE_EXECUTABLES = {
    "dgemm": join(PRK_NATIVE_BUILD, "MPI1", "DGEMM", "dgemm"),
    "nstream": join(PRK_NATIVE_BUILD, "MPI1", "Nstream", "nstream"),
    "random": join(PRK_NATIVE_BUILD, "MPI1", "Random", "random"),
    "reduce": join(PRK_NATIVE_BUILD, "MPI1", "Reduce", "reduce"),
    "sparse": join(PRK_NATIVE_BUILD, "MPI1", "Sparse", "sparse"),
    "stencil": join(PRK_NATIVE_BUILD, "MPI1", "Stencil", "stencil"),
    "global": join(PRK_NATIVE_BUILD, "MPI1", "Synch_global", "global"),
    "p2p": join(PRK_NATIVE_BUILD, "MPI1", "Synch_p2p", "p2p"),
    "transpose": join(PRK_NATIVE_BUILD, "MPI1", "Transpose", "transpose"),
}

PRK_STATS = {
    "dgemm": (),
    "nstream": ("Avg time (s)", "Rate (MB/s)"),
    "random": (),
    "reduce": (),
    "sparse": (),
    "stencil": (),
    "global": (),
    "p2p": (),
    "transpose": (),
}


@task
def invoke_prk(ctx, func, native=False, iface=None, np=10):
    if func not in PRK_CMDLINE:
        print("Invalid PRK function {}".format(func))
        return 1

    cmdline = PRK_CMDLINE[func]

    if native:
        executable = PRK_NATIVE_EXECUTABLES[func]
        cmd_out = mpi_run(executable, iface=iface, cmdline=cmdline, np=np)
        cmd_out = cmd_out.decode()
        print(cmd_out)
    else:
        host, port = get_invoke_host_port()
        cmd_out = invoke_impl(FAASM_USER, func, cmdline=cmdline, host=host, port=port, mpi_world_size=np)

    _parse_prk_out(func, cmd_out)


def _parse_prk_out(func, cmd_out):
    stats = PRK_STATS.get(func)

    if not stats:
        print("No stats for {}".format(func))
        return

    print("----- {} stats -----".format(func))

    for stat in stats:
        spilt_str = "{}: ".format(stat)

        stat_val = [c for c in cmd_out.split(spilt_str) if c.strip()]
        if not stat_val:
            print("{} = MISSING".format(stat))
            continue

        stat_val = stat_val[1]
        stat_val = stat_val.split(" ")[0]
        stat_val = float(stat_val)

        print("{} = {}".format(stat, stat_val))
