
from os.path import join
from subprocess import check_output, call

from invoke import task

from faasmcli.util.env import BENCHMARK_BUILD


@task
def max_threads(ctx):
    """
    Run max threads check
    """
    print("---- ulimit ----")
    call("ulimit -aS", shell=True)

    print("\n---- sysctl ----")
    sysctl_max_threads = check_output("sysctl -n kernel.threads-max", shell=True).decode("utf-8")
    print("kernel.threads-max = {}".format(sysctl_max_threads))

    print("\n---- observed ----")
    script_path = join(BENCHMARK_BUILD, "bin", "max_thread_experiment")
    call(script_path, shell=True)

