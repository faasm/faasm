from os.path import join
from os.path import join
from subprocess import check_output, call

from invoke import task

from tasks.util.env import BENCHMARK_BUILD


@task
def max_threads(ctx):
    script_path = join(BENCHMARK_BUILD, "bin", "max_thread_experiment")

    call(script_path, shell=True)
