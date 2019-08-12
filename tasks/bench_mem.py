from decimal import Decimal
from multiprocessing import Process
from os.path import join
from subprocess import call
from time import sleep

from invoke import task

from tasks.util.env import PROJ_ROOT, BENCHMARK_BUILD
from tasks.util.memory import get_total_memory_for_pid, get_total_memory_for_pids
from tasks.util.process import get_docker_parent_pids, get_pid_for_name

OUTPUT_FILE = "/tmp/runtime-bench-mem.csv"


def _exec_cmd(cmd_str):
    print(cmd_str)
    ret_code = call(cmd_str, shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        raise RuntimeError("Command failed: {} ({})".format(cmd_str, ret_code))


@task
def bench_mem(ctx, runtime=None):
    repeats = 3

    # Sleep time here needs to be around 0.5/0.75x the sleep of the process so we catch when everything is up
    faasm_bench = (
        "faasm",
        join(BENCHMARK_BUILD, "bin", "bench_mem"),
        "bench_mem",
        [2000, 1800, 1600, 1400, 1200, 1000, 800, 600, 400, 200, 1],
        15
    )

    docker_bench = (
        "docker",
        "./bin/docker_noop_mem.sh",
        None,
        [240, 220, 200, 180, 160, 140, 120, 100, 80, 60, 40, 1],
        70,
    )

    thread_bench = (
        "thread",
        join(BENCHMARK_BUILD, "bin", "thread_bench_mem"),
        "thread_bench_mem",
        [2000, 1800, 1600, 1400, 1200, 1000, 800, 600, 400, 200, 1],
        5
    )

    if runtime == "faasm":
        benches = [faasm_bench]
    elif runtime == "docker":
        benches = [docker_bench]
    else:
        benches = [faasm_bench, docker_bench, thread_bench]

    csv_out = open(OUTPUT_FILE, "w")
    csv_out.write("Runtime,Measure,Value,Workers,ValuePerWorker\n")

    for r in range(0, repeats):
        for bench_name, cmd, process_name, n_workers_list, sleep_time in benches:
            for n_workers in n_workers_list:
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

                if bench_name == "docker":
                    docker_pids = get_docker_parent_pids()
                    print("Measuring memory of docker processes {}".format(docker_pids))
                    mem_total = get_total_memory_for_pids(docker_pids)
                else:
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
