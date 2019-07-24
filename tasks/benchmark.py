from decimal import Decimal
from subprocess import call
from tempfile import NamedTemporaryFile

import pandas as pd
from invoke import task

from tasks.env import PROJ_ROOT

# Absolute path to time required for bash
TIME_BINARY = "/usr/bin/time"
OUTPUT_FILE = "/tmp/runtime-bench.csv"

BENCHMARKS = {
    "faasm": {
        "cmd": "./cmake-build-release/bin/runtime_bench",
    },
    "docker": {
        "cmd": "./bin/docker_noop.sh",
    },
    "thread": {
        "cmd": "./cmake-build-release/bin/thread_bench",
    },
}

TIME_LABELS = {
    ("Elapsed (wall clock) time (h:mm:ss or m:ss)", "elapsed_seconds"),
    ("Maximum resident set size (kbytes)", "max_resident_kb"),
}


class RuntimeBenchRunner:

    def __init__(self):
        self.n_workers = 0
        self.n_iterations = 0

        self.csv_out = open(OUTPUT_FILE, "w")
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

    def do_run(self, n_workers, n_iterations):
        self.n_workers = n_workers
        self.n_iterations = n_iterations

        for runtime_name, bench_details in BENCHMARKS.items():
            print("\n------ {} ------\n".format(runtime_name))

            print("Running for runtime: {}".format(runtime_name))
            self._do_time(runtime_name, bench_details)
            self._do_perf(runtime_name, bench_details)

    def _do_perf(self, runtime_name, bench_details):
        # Build the command with output to temp file
        out_file = NamedTemporaryFile()
        cmd = [
            "perf", "stat", "-x, -e cycles,instructions -a",
            "-o", out_file.name,
            bench_details["cmd"],
            str(self.n_workers),
            str(self.n_iterations),
        ]

        cmd_str = " ".join(cmd)
        print(cmd_str)

        # Execute the command
        call(
            cmd_str,
            cwd=PROJ_ROOT,
            shell=True,
        )

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

    def _do_time(self, runtime_name, bench_details):
        # Build the command with output to temp file
        out_file = NamedTemporaryFile()
        cmd = [
            TIME_BINARY,
            "-v",
            "-o", out_file.name,
            bench_details["cmd"],
            str(self.n_workers),
            str(self.n_iterations),
        ]

        cmd_str = " ".join(cmd)
        print(cmd_str)

        # Execute the command
        call(
            cmd_str,
            cwd=PROJ_ROOT,
            shell=True,
        )

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
def runtime_bench(ctx):
    n_workers = [1, 2, 3, 4, 5, 6, 7]
    n_iterations = 30
    repeats = 2

    runner = RuntimeBenchRunner()

    # Repeat runs, once for each worker
    for w in n_workers:
        for i in range(0, repeats):
            runner.do_run(w, n_iterations)

    runner.done()
