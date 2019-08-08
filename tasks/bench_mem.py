from decimal import Decimal
from multiprocessing import Process
from subprocess import call
from time import sleep

from invoke import task

from tasks.util.env import PROJ_ROOT
from tasks.util.memory import get_pid_for_name, get_total_memory_for_pid

OUTPUT_FILE = "/tmp/runtime-bench-mem.csv"


def _exec_cmd(cmd_str):
    print(cmd_str)
    ret_code = call(cmd_str, shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        raise RuntimeError("Command failed: {} ({})".format(cmd_str, ret_code))


@task
def bench_mem(ctx):
    n_workers_list = [5, 10]

    # Sleep time here needs to be around half the sleep of the process so we catch it in the middle
    benches = [
        ("faasm", "./cmake-build-release/bin/bench_mem", None, 5),
        ("docker", "./bin/docker_noop_mem.sh", "dockerd", 15),
        ("thread", "./cmake-build-release/bin/thread_bench_mem", None, 5),
    ]

    csv_out = open(OUTPUT_FILE, "w")
    csv_out.write(
        "Runtime,Measure,Value,Workers,ValuePerWorker\n")

    for n_workers in n_workers_list:
        for bench_name, cmd, parent_proc, sleep_time in benches:
            print("BENCH: {} - {} workers".format(bench_name, n_workers))

            # Launch the process in the background
            cmd = [
                cmd,
                str(n_workers),
            ]
            cmd_str = " ".join(cmd)

            # Launch subprocess
            sleep_proc = Process(target=_exec_cmd, args=[cmd_str])
            sleep_proc.start()

            process_pid = sleep_proc.pid
            print("Launched background process {} ({})".format(process_pid, cmd_str))

            sleep(sleep_time)

            if parent_proc:
                process_pid = get_pid_for_name(parent_proc)
                print("Measuring parent process {} ({})".format(process_pid, parent_proc))

            # Get the memory of the given process
            mem_total = get_total_memory_for_pid(process_pid)
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
