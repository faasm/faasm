from decimal import Decimal
from multiprocessing import Process
from subprocess import call
from tempfile import NamedTemporaryFile

from invoke import task
from time import sleep

from tasks.util.env import PROJ_ROOT
from tasks.util.memory import get_total_memory_for_pid, get_pid_for_name

# Absolute path to time required for bash
TIME_BINARY = "/usr/bin/time"
SPEED_OUTPUT_FILE = "/tmp/runtime-bench-speed.csv"
RESOURCE_OUTPUT_FILE = "/tmp/runtime-bench-resource.csv"

BENCHMARKS = {
    "faasm": {
        "cmd": "./cmake-build-release/bin/runtime_bench",
        "iteration_multiplier": 100,
    },
    "docker": {
        "cmd": "./bin/docker_noop.sh",
        "parent_proc": "dockerd",
        "iteration_multiplier": 1,
    },
    "thread": {
        "cmd": "./cmake-build-release/bin/thread_bench",
        "iteration_multiplier": 100,
    },
}

TIME_LABELS = {
    ("Elapsed (wall clock) time (h:mm:ss or m:ss)", "elapsed_seconds"),
    # ("Maximum resident set size (kbytes)", "max_resident_kb"),
}


class RuntimeBenchRunner:
    def __init__(self, output_file):
        self.n_workers = 0
        self.n_iterations = 0

        self.csv_out = open(output_file, "w")
        self.csv_out.write(
            "Workers,Iterations,Runtime,Measure,Value,ValuePerIteration,ValuePerWorker,ValuePerIterationPerWorker\n")

    def done(self):
        self.csv_out.close()

    def _write_stat(self, bench_name, metric, value):
        if isinstance(value, str):
            value = Decimal(value)

        value_per_iteration = value / self.n_iterations
        value_per_worker = value / self.n_workers
        value_per_iteration_per_worker = value_per_iteration / self.n_workers

        self.csv_out.write("{},{},{},{},{},{},{},{}\n".format(
            self.n_workers, self.n_iterations,
            bench_name, metric, value,
            value_per_iteration, value_per_worker,
            value_per_iteration_per_worker
        ))

    def do_mem_run(self, n_workers):
        self.n_workers = n_workers
        self.n_iterations = 1

        for runtime_name, bench_details in BENCHMARKS.items():
            print("\n------ {} ------\n".format(runtime_name))

            print("Running memory benchmark: {}".format(runtime_name))
            self._do_mem(runtime_name, bench_details)

    def do_time_run(self, n_iterations):
        self.n_workers = 1

        for runtime_name, bench_details in BENCHMARKS.items():
            print("\n------ {} ------\n".format(runtime_name))

            # Scale up iterations
            self.n_iterations = n_iterations * bench_details["iteration_multiplier"]

            print("Running time benchmark: {}".format(runtime_name))
            self._do_time_seconds(runtime_name, bench_details)
            self._do_cpu_cycles(runtime_name, bench_details)

    def _exec_cmd(self, cmd_str):
        print(cmd_str)
        ret_code = call(cmd_str, shell=True, cwd=PROJ_ROOT)

        if ret_code != 0:
            raise RuntimeError("Command failed: {}".format(ret_code))

    def _do_mem(self, runtime_name, bench_details):
        # Launch the process in the background
        cmd = [
            bench_details["cmd"],
            str(self.n_workers),
            str(self.n_iterations),
            "1",
        ]
        cmd_str = " ".join(cmd)

        # Launch subprocess
        sleep_proc = Process(target=self._exec_cmd, args=[cmd_str])
        sleep_proc.start()

        parent_proc = bench_details.get("parent_proc")
        process_pid = sleep_proc.pid
        print("Launched background process {} ({})".format(process_pid, cmd_str))
        if parent_proc:
            process_pid = get_pid_for_name(parent_proc)
            print("Measuring parent process {} ({})".format(process_pid, parent_proc))

        # Wait a bit. Needs to be enough to let Docker spawn all containers
        sleep_time = 5
        if runtime_name == "docker":
            sleep_time = 15

        sleep(sleep_time)

        # Get the memory of the given process
        mem_total = get_total_memory_for_pid(process_pid)
        for label, data in zip(mem_total.get_labels(), mem_total.get_data()):
            self._write_stat(runtime_name, label, data)

        self.csv_out.flush()

        # Rejoin the background process
        sleep_proc.join()

    def _do_cpu_cycles(self, runtime_name, bench_details):
        # Build the command with output to temp file
        out_file = NamedTemporaryFile()
        cmd = [
            "perf", "stat", "-x, -e cycles,instructions -a",
            "-o", out_file.name,
            bench_details["cmd"],
            str(self.n_workers),
            str(self.n_iterations),
            "0",
        ]

        cmd_str = " ".join(cmd)
        self._exec_cmd(cmd_str)

        # Read in input
        with open(out_file.name, "r") as fh:
            for perf_line in fh:
                perf_line = perf_line.strip()
                if not perf_line or perf_line.startswith("#"):
                    continue

                parts = perf_line.split(",")
                metric = parts[2]
                value = parts[0]

                if metric == "cycles":
                    # Amortize over iterations
                    value = Decimal(value) / self.n_iterations
                    self._write_stat(runtime_name, "cpu_cycles", value)

        self.csv_out.flush()

    def _do_time_seconds(self, runtime_name, bench_details):
        # Build the command with output to temp file
        out_file = NamedTemporaryFile()
        cmd = [
            TIME_BINARY,
            "-v",
            "-o", out_file.name,
            bench_details["cmd"],
            str(self.n_workers),
            str(self.n_iterations),
            "0",
        ]

        cmd_str = " ".join(cmd)
        self._exec_cmd(cmd_str)

        # Parse time stats as dictionary
        time_stats = dict()
        with open(out_file.name, "r") as fh:
            for time_line in fh:
                if not time_line:
                    continue

                time_line = time_line.strip()

                # Wall clock time is messed up as it has colons in the string as well as for the separator
                # Assume format of mm:ss.ss
                if time_line.startswith("Elapsed (wall"):
                    # Extract the label
                    label = ":".join(time_line.split(":")[:-2])

                    # Parse into seconds
                    minute_str, seconds_str = time_line.split(":")[-2:]
                    value = (Decimal(minute_str) * 60) + Decimal(seconds_str)
                else:
                    label, value = time_line.split(":")
                    value = value.strip()

                time_stats[label.strip()] = value

        # Check return code
        if time_stats["Exit status"] != "0":
            raise RuntimeError("Time command failed. Time stats: {}".format(time_stats))

        # Map time labels to output labels
        for time_label, output_label in TIME_LABELS:
            value = time_stats[time_label]
            self._write_stat(runtime_name, output_label, value)

        self.csv_out.flush()


@task
def runtime_bench_time(ctx):
    n_iterations = 10

    runner = RuntimeBenchRunner(SPEED_OUTPUT_FILE)

    runner.do_time_run(n_iterations)

    runner.done()


@task
def runtime_bench_mem(ctx):
    n_workers = [1, 10, 20, 30, 40, 50]
    repeats = 2

    runner = RuntimeBenchRunner(RESOURCE_OUTPUT_FILE)

    # Repeat runs, once for each worker
    for n_worker in n_workers:
        for i in range(0, repeats):
            runner.do_mem_run(n_worker)

    runner.done()


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
