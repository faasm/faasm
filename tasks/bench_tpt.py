from os import makedirs
from os.path import join, exists
from subprocess import call
from decimal import Decimal

import numpy as np
from invoke import task

from tasks.util.env import RESULT_DIR, PROJ_ROOT, set_benchmark_env, BENCHMARK_BUILD

OUTPUT_FILE = join(RESULT_DIR, "runtime-bench-tpt.csv")


def _exec_cmd(cmd_str):
    print(cmd_str)
    set_benchmark_env()
    ret_code = call(cmd_str, shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        raise RuntimeError("Command failed: {}".format(ret_code))


def _numbers_from_file(file_path):
    values = [float(l.split(" ")[0]) for l in open(file_path) if l.strip()]
    return values


def _write_tpt_lat(run_num, runtime_name, target_tpt, csv_out, tolerance=0):
    tpt_file = "/tmp/{}_tpt.log".format(runtime_name)
    lat_file = "/tmp/{}_lat.log".format(runtime_name)
    duration_file = "/tmp/{}_duration.log".format(runtime_name)

    times = _numbers_from_file(tpt_file)
    lats = _numbers_from_file(lat_file)
    durations = _numbers_from_file(duration_file)

    n_times = len(times)
    n_lats = len(lats)
    n_diff = abs(n_times - n_lats)
    msg = "Requests and latencies count doesn't match within tolerance ({} vs {})".format(n_times, n_lats)
    assert n_diff <= tolerance, msg

    assert len(durations) == 1, "Found multiple durations"
    duration = float(durations[0])

    # Throughput should be per second
    tpt = n_times / (duration / 1000)

    # Other stats
    lat_median = np.median(lats)
    lat_90 = np.percentile(lats, 90)
    lat_99 = np.percentile(lats, 99)

    csv_out.write(
        "{},{},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f}\n".format(
            run_num,
            runtime_name,
            duration,
            target_tpt,
            tpt,
            lat_median,
            lat_90,
            lat_99,
        ))

    csv_out.flush()


@task
def bench_tpt(ctx, runtime=None):
    repeats = 2

    if not exists(RESULT_DIR):
        makedirs(RESULT_DIR)

    csv_out = open(OUTPUT_FILE, "w")
    csv_out.write("RunNum,Runtime,Duration,TargetThroughput,Throughput,LatencyMed,Latency90,Latency99\n")
    csv_out.flush()

    set_benchmark_env()

    for r in range(repeats):
        print("Throughput benchmark repeat {}".format(r))

        if runtime == "docker" or runtime is None:
            # NOTE - Docker tpt script needs delay in a seconds string and runtime in millis
            runs = [
                ("10", "30000"),
                ("8", "30000"),
                ("6", "25000"),
                ("4", "20000"),
                ("2", "15000"),
                ("1.5", "15000"),
                ("1.25", "15000"),
                ("1", "15000"),
                ("0.75", "15000"),
                ("0.5", "15000"),
                ("0.25", "15000"),
                ("0.15", "15000"),
            ]

            for delay, runtime_length in runs:
                # Run the bench
                cmd = [
                    join(PROJ_ROOT, "bin", "docker_tpt.sh"),
                    delay,
                    runtime_length,
                ]
                cmd_str = " ".join(cmd)

                _exec_cmd(cmd_str)

                # Write the result
                target_tpt = Decimal("1") / Decimal(delay)
                _write_tpt_lat(r, "docker", target_tpt, csv_out, tolerance=5)

        if runtime == "faasm" or runtime is None:
            # NOTE: both are in microseconds
            runs = [
                ("10000000", "30000"),
                ("6000000", "20000"),
                ("2000000", "15000"),
                ("1000000", "15000"),
                ("500000", "15000"),
                ("250000", "15000"),
                ("100000", "10000"),
                ("50000", "10000"),
                ("25000", "10000"),
                ("10000", "10000"),
                ("5000", "10000"),
                ("1000", "10000"),
                ("750", "10000"),
            ]

            for delay, runtime_length in runs:
                # Run the bench
                cmd = [
                    join(BENCHMARK_BUILD, "bin", "bench_tpt"),
                    delay,
                    runtime_length,
                ]
                cmd_str = " ".join(cmd)

                _exec_cmd(cmd_str)

                # Write the result
                target_tpt = (Decimal("1000000")) / Decimal(delay)
                _write_tpt_lat(r, "faasm", target_tpt, csv_out)

    csv_out.close()
