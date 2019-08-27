from decimal import Decimal
from os.path import join
from subprocess import call
from tempfile import NamedTemporaryFile

from invoke import task

from tasks.util.env import PROJ_ROOT, BENCHMARK_BUILD, RESULT_DIR


@task
def bench_tpt(ctx):
    pass
