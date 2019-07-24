from decimal import Decimal
from subprocess import call
from tempfile import NamedTemporaryFile

import pandas as pd
from invoke import task

from tasks.env import PROJ_ROOT

# Absolute path required for bash
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
    ("Elapsed (wall clock) time (h:mm:ss or m:ss)", "elapsed_seconds", True),
    ("Maximum resident set size (kbytes)", "max_resident_kb", False),
}


def _do_perf(bench_name, bench_details, out_csv, n_workers, n_iterations, repeat_count):
    # Build the command with output to temp file
    out_file = NamedTemporaryFile()
    cmd = [
        "perf", "stat", "-x, -e cycles,instructions -a",
        "-o", out_file.name,
        bench_details["cmd"],
        str(n_workers),
        str(n_iterations),
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
                value = Decimal(value) / n_iterations
                out_csv.write("{},{},{},{}\n".format(repeat_count, bench_name, "cpu_cycles", value))

    out_csv.flush()


def _do_time(bench_name, bench_details, out_csv, n_workers, n_iterations, repeat_count):
    # Build the command with output to temp file
    out_file = NamedTemporaryFile()
    cmd = [
        TIME_BINARY,
        "-v",
        "-o", out_file.name,
        bench_details["cmd"],
        str(n_workers),
        str(n_iterations),
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
    for time_label, output_label, divide_by_time in TIME_LABELS:
        value = time_stats[time_label]

        # Amortize over iterations if required
        if divide_by_time:
            value = Decimal(value) / n_iterations

        out_csv.write("{},{},{},{}\n".format(repeat_count, bench_name, output_label, value))

    out_csv.flush()


@task
def runtime_bench(ctx):
    csv_out = open(OUTPUT_FILE, "w")

    csv_out.write("Run,Runtime,Measure,Value\n")

    n_workers = 1
    n_iterations = 5
    n_repeats = 3

    for bench_name, bench_details in BENCHMARKS.items():
        print("\n------ {} ------\n".format(bench_name))

        for i in range(n_repeats):
            print("Running bench: {} (repeat {})".format(bench_name, i))
            _do_time(bench_name, bench_details, csv_out, n_workers, n_iterations, i)
            _do_perf(bench_name, bench_details, csv_out, n_workers, n_iterations, i)

    csv_out.close()


@task
def analyse_runtime_bench(ctx):
    bench_file = "/tmp/runtime-bench.csv"

    data = pd.read_csv(bench_file)

    print("----- Results -----")
    print("| {:<10} | {:<20} | {:<15} | {:<15} | {:<9} | {:<9} |"
          .format("Runtime", "Measure", "Value", "Stddev", "vs. Faasm", "vs. thread"))

    _extract_stat(data, "cpu_cycles")
    _extract_stat(data, "elapsed_seconds")
    _extract_stat(data, "max_resident_kb")


def _extract_stat(full_data, measure_name):
    filtered = full_data[full_data["Measure"] == measure_name]
    if filtered.empty:
        raise RuntimeError("No data for {}".format(measure_name))

    runtimes = filtered["Runtime"].unique()

    grouped = filtered.groupby(["Runtime"])
    median = grouped.median()
    stddev = grouped.std()

    faasm_med = median.loc["faasm"]["Value"]
    native_med = median.loc["thread"]["Value"]

    for runtime in runtimes:
        this_med = median.loc[runtime]["Value"]
        this_std = stddev.loc[runtime]["Value"]

        faasm_overhead = _pct_diff(this_med, faasm_med)
        native_overhead = _pct_diff(this_med, native_med)

        # Print data
        print("| {:<10} | {:<20} | {:<15.4f} | {:<15.4f} | {:>8}% | {:>8}% |"
              .format(runtime, measure_name, this_med, this_std, faasm_overhead, native_overhead))


def _pct_diff(value, bench):
    if bench == 0:
        return "inf"

    return int(((value - bench) / bench) * 100)
