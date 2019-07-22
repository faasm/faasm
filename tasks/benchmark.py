from pprint import pprint
from subprocess import check_output
from tempfile import NamedTemporaryFile

from invoke import task

from tasks.env import PROJ_ROOT, WASM_DIR

BENCHMARKS = {
    "faasm": {
        "cwd": "/usr/local/code/faasm",
        "cmd": "./cmake-build-debug/bin/func_runner demo echo",
    }
}


def _do_perf(bench_details):
    pass


def _do_time(bench_name, bench_details, out_csv):
    # Build the command with output to temp file
    out_file = NamedTemporaryFile()
    cmd = [
        "time",
        "-v",
        "-o", out_file.name,
        bench_details["cmd"],
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)

    # Execute the command
    check_output(
        cmd_str,
        cwd=bench_details["cwd"],
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


    pprint(time_stats)

    out_file.close()

@task
def runtime_bench(ctx):
    csv_path = "/tmp/runtime-bench.csv"
    csv_out = open(csv_path, "w")

    for bench_name, bench_details in BENCHMARKS.items():
        print("Running bench: {}".format(bench_name))
        _do_time(bench_name, bench_details, csv_out)
        _do_perf(bench_name, bench_details, csv_out)

    csv_out.close()
