import numpy as np
from os import makedirs
from os.path import join, exists
from subprocess import call

from invoke import task

from tasks.util.env import RESULT_DIR, PROJ_ROOT, set_benchmark_env, BENCHMARK_BUILD

OUTPUT_FILE = join(RESULT_DIR, "runtime-bench-tpt.csv")


def _exec_cmd(cmd_str):
    print(cmd_str)
    set_benchmark_env()
    ret_code = call(cmd_str, shell=True, cwd=PROJ_ROOT)

    if ret_code != 0:
        raise RuntimeError("Command failed: {}".format(ret_code))


def _write_tpt_lat(runtime_name, csv_out):
    tpt_file = "/tmp/{}_tpt.log".format(runtime_name)
    lat_file = "/tmp/{}_lat.log".format(runtime_name)

    times = [int(t.split(" ")[0]) for t in open(tpt_file)]
    lats = [float(l.split(" ")[0]) for l in open(lat_file)]

    n_times = len(times)
    n_lats = len(lats)
    msg = "Requests and latencies count doesn't match ({} vs {})".format(n_times, n_lats)
    assert n_times == n_lats, msg

    duration = np.max(times)
    tpt = duration / n_times
    lat_median = np.median(lats)
    lat_90 = np.percentile(lats, 90)
    lat_99 = np.percentile(lats, 99)

    csv_out.write("{},{},{},{},{}\n".format(runtime_name, tpt, lat_median, lat_90, lat_99))


@task
def bench_tpt(ctx, runtime=None):
    if not exists(RESULT_DIR):
        makedirs(RESULT_DIR)

    csv_out = open(OUTPUT_FILE, "w")
    csv_out.write("Runtime,Throughput,LatencyMed,Latency90,Latency99\n")

    set_benchmark_env()

    if runtime == "faasm" or runtime is None:
        # NOTE: both are in millis
        delays = ["2000", "1000", "500", "250", "125", "72", "36", "18", "9", "1"]
        runtime = "20000"

        for delay in delays:
            # Run the bench
            cmd = [
                join(BENCHMARK_BUILD, "bin", "bench_tpt"),
                delay,
                runtime,
            ]
            cmd_str = " ".join(cmd)

            _exec_cmd(cmd_str)

            # Write the result
            _write_tpt_lat("faasm", csv_out)

    elif runtime == "docker" or runtime is None:
        # NOTE - Docker tpt script needs delay in a seconds string and runtime in millis
        delays = ["2", "1", "0.5", "0.25", "0.12", "0.06"]
        runtime = "20000"

        for delay in delays:
            # Run the bench
            cmd = [
                join(PROJ_ROOT, "bin", "docker_tpt.sh"),
                delay,
                runtime,
            ]
            cmd_str = " ".join(cmd)

            _exec_cmd(cmd_str)

            # Write the result
            _write_tpt_lat("docker", csv_out)

    csv_out.close()
