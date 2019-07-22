from subprocess import check_output
from tempfile import NamedTemporaryFile

from invoke import task

from tasks.env import PROJ_ROOT

N_ITERATIONS = 1000

# Absolute path required for bash
TIME_BINARY = "/usr/bin/time"

BENCHMARKS = {
    "faasm": {
        "cmd": "./cmake-build-release/bin/runtime_bench",
    },
    "docker": {
        "cmd": "./bin/docker_noop.sh",
    }
}

TIME_LABELS = {
    "Elapsed (wall clock) time (h:mm:ss or m:ss)": "elapsed",
    "Maximum resident set size (kbytes)": "max_resident_kb",
}


def _do_perf(bench_name, bench_details, out_csv):
    # Build the command with output to temp file
    out_file = NamedTemporaryFile()
    cmd = [
        "perf", "stat", "-x, -e cycles,instructions -a",
        "-o", out_file.name,
        bench_details["cmd"],
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)

    # Execute the command
    check_output(
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
                out_csv.write("{},{},{}\n".format(bench_name, "cpu_cycles", value))

    out_file.close()


def _do_time(bench_name, bench_details, out_csv):
    # Build the command with output to temp file
    out_file = NamedTemporaryFile()
    cmd = [
        TIME_BINARY,
        "-v",
        "-o", out_file.name,
        bench_details["cmd"],
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)

    # Execute the command
    check_output(
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
                value = ":".join(time_line.split(":")[-2:])
                label = ":".join(time_line.split(":")[:-2])
            else:
                label, value = time_line.split(":")

            time_stats[label.strip()] = value.strip()

    # Check return code
    if time_stats["Exit status"] != "0":
        raise RuntimeError("Time command failed. Time stats: {}".format(time_stats))

    # Map time labels to output labels
    for time_label, output_label in TIME_LABELS.items():
        value = time_stats[time_label]
        out_csv.write("{},{},{}\n".format(bench_name, output_label, value))

    out_file.close()


@task
def runtime_bench(ctx):
    csv_path = "/tmp/runtime-bench.csv"
    csv_out = open(csv_path, "w")

    csv_out.write("Runtime,Measure,Value\n")

    for bench_name, bench_details in BENCHMARKS.items():
        print("Running bench: {}".format(bench_name))
        _do_time(bench_name, bench_details, csv_out)
        _do_perf(bench_name, bench_details, csv_out)

    csv_out.close()
