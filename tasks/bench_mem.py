from decimal import Decimal
from multiprocessing import Process
from os.path import exists, join
from os import makedirs
from subprocess import call, check_output
from time import sleep

from invoke import task

from tasks.util.env import PROJ_ROOT, BENCHMARK_BUILD, RESULT_DIR
from tasks.util.memory import get_total_memory_for_pid, get_total_memory_for_pids
from tasks.util.process import get_docker_parent_pids, get_pid_for_name

OUTPUT_FILE = join(RESULT_DIR, "runtime-bench-mem.csv")


def _exec_cmd(cmd_str):
    print(cmd_str)
    ret_code = call(cmd_str, shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        raise RuntimeError("Command failed: {} ({})".format(cmd_str, ret_code))


@task
def bench_mem(ctx, runtime=None):
    if not exists(RESULT_DIR):
        makedirs(RESULT_DIR)

    csv_out = open(OUTPUT_FILE, "w")
    csv_out.write("Runtime,Measure,Value,Workers,ValuePerWorker\n")

    for repeat in range(0, 3):
        if runtime == "faasm" or runtime is None:
            # Sleep time here needs to be around 0.5/0.75x the sleep of the process so we catch when everything is up
            for n_workers in [1, 200, 400, 600, 800, 1000, 1200, 1400, 1600, 1800, 2000]:
                _run_sleep_bench(
                    "faasm",
                    n_workers,
                    join(BENCHMARK_BUILD, "bin", "bench_mem"),
                    15,
                    "bench_mem",
                    csv_out
                )

        if runtime == "docker" or runtime is None:
            for n_workers in [2000]:
                _run_docker_bench(
                    n_workers,
                    csv_out,
                )

        if runtime is None:
            for n_workers in [1, 200, 400, 600, 800, 1000, 1200, 1400, 1600, 1800, 2000]:
                _run_sleep_bench(
                    "thread",
                    n_workers,
                    join(BENCHMARK_BUILD, "bin", "thread_bench_mem"),
                    5,
                    "thread_bench_mem",
                    csv_out
                )


def _run_sleep_bench(bench_name, n_workers, cmd, sleep_time, process_name, csv_out):
    print("BENCH: {} - {} workers".format(bench_name, n_workers))

    # Launch the process in the background
    cmd_str = [
        cmd,
        str(n_workers),
    ]
    cmd_str = " ".join(cmd_str)

    # Launch subprocess
    sleep_proc = Process(target=_exec_cmd, args=[cmd_str])
    sleep_proc.start()
    sleep(sleep_time)

    pid = get_pid_for_name(process_name)
    print("Measuring memory of process {}".format(pid))
    mem_total = get_total_memory_for_pid(pid)

    for label, value in zip(mem_total.get_labels(), mem_total.get_data()):
        csv_out.write("{},{},{},{},{}\n".format(
            bench_name,
            label,
            value,
            n_workers,
            Decimal(value) / n_workers,
        ))

    csv_out.flush()

    # Rejoin the background process
    sleep_proc.join()


def _run_docker_bench(n_workers, csv_out):
    # Kick off the process
    # Do this in batches otherwise things can get overloaded
    batch_size = 50
    batch_remainder = n_workers % batch_size
    n_batches = n_workers // batch_size + (batch_remainder > 0)
    for b in range(n_batches):
        if (b == n_batches - 1) and (batch_remainder > 0):
            this_batch_size = batch_remainder
        else:
            this_batch_size = batch_size

        start_cmd = "./bin/docker_mem_start.sh {}".format(this_batch_size)
        print("Kicking off Docker batch size {}".format(this_batch_size))
        start_ret_code = call(start_cmd, shell=True, cwd=PROJ_ROOT)
        if start_ret_code != 0:
            raise RuntimeError("Start Docker benchmark failed")

    # Get total mem and write
    pids = get_docker_parent_pids()
    mem_total = get_total_memory_for_pids(pids)

    for label, value in zip(mem_total.get_labels(), mem_total.get_data()):
        csv_out.write("{},{},{},{},{}\n".format(
            "docker",
            label,
            value,
            n_workers,
            Decimal(value) / n_workers,
        ))

    csv_out.flush()

    # Finish
    end_cmd = "./bin/docker_mem_end.sh"
    end_ret_code = call(end_cmd, shell=True, cwd=PROJ_ROOT)
    if end_ret_code != 0:
        raise RuntimeError("Ending Docker benchmark failed")


@task
def pid_mem(ctx, pid):
    pid = int(pid)
    _print_pid_mem(pid)


@task
def plot_pid_mem(ctx, pid):
    pid = int(pid)
    _plot_pid_mem(pid)


@task
def proc_mem(ctx, proc_name):
    pid = get_pid_for_name(proc_name)
    _print_pid_mem(pid)


@task
def plot_proc_mem(ctx, proc_name):
    pid = get_pid_for_name(proc_name)
    _plot_pid_mem(pid)


def _plot_pid_mem(pid):
    mem_total = get_total_memory_for_pid(pid)
    mem_total.plot()


def _print_pid_mem(pid):
    mem_total = get_total_memory_for_pid(pid)
    mem_total.print()
