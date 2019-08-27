from os import makedirs
from os.path import join, exists
from subprocess import call

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


def _write_tpt_lat(run_num, runtime_name, csv_out, tolerance=0):
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
        "{},{},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f}\n".format(run_num, runtime_name, duration, tpt, lat_median, lat_90,
                                                            lat_99))

    csv_out.flush()


@task
def bench_tpt(ctx, runtime=None):
    repeats = 3

    if not exists(RESULT_DIR):
        makedirs(RESULT_DIR)

    csv_out = open(OUTPUT_FILE, "w")
    csv_out.write("RunNum,Runtime,Duration,Throughput,LatencyMed,Latency90,Latency99\n")
    csv_out.flush()

    set_benchmark_env()

    for r in range(repeats):
        print("Throughput benchmark repeat {}".format(r))

        if runtime == "docker" or runtime is None:
            # NOTE - Docker tpt script needs delay in a seconds string and runtime in millis
            delays = ["2", "1.5", "1.25", "1", "0.75", "0.5", "0.25"]
            runtime_length = "15000"

            for delay in delays:
                # Run the bench
                cmd = [
                    join(PROJ_ROOT, "bin", "docker_tpt.sh"),
                    delay,
                    runtime_length,
                ]
                cmd_str = " ".join(cmd)

                _exec_cmd(cmd_str)

                # Write the result
                _write_tpt_lat(r, "docker", csv_out, tolerance=5)

        if runtime == "faasm" or runtime is None:
            # NOTE: both are in millis
            delays = ["2000", "1500", "1000", "500", "250", "125", "75", "50", "25", "15", "10", "5", "1"]
            runtime_length = "10000"

            for delay in delays:
                # Run the bench
                cmd = [
                    join(BENCHMARK_BUILD, "bin", "bench_tpt"),
                    delay,
                    runtime_length,
                ]
                cmd_str = " ".join(cmd)

                _exec_cmd(cmd_str)

                # Write the result
                _write_tpt_lat(r, "faasm", csv_out)

    csv_out.close()
