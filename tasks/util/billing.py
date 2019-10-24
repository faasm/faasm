from collections import defaultdict
from decimal import Decimal
from os import listdir
from os.path import join, basename, normpath
from subprocess import call

import numpy as np

from tasks.util.env import ANSIBLE_ROOT


def _ansible_playbook(playbook_name):
    cmd = [
        "ansible-playbook",
        "-i inventory/billing.yml",
        "{}.yml".format(playbook_name)
    ]

    cmd_str = " ".join(cmd)
    print(cmd_str)
    res = call(cmd_str, cwd=ANSIBLE_ROOT, shell=True)
    if res != 0:
        print("Ansible command failed: {}".format(cmd_str))
        raise RuntimeError("Ansible command failed: {}".format(cmd_str))


def start_billing():
    _ansible_playbook("billing_start")


def pull_billing():
    _ansible_playbook("billing_pull")


def parse_billing(result_dir, summary_out_dir):
    run_name = basename(normpath(summary_out_dir))

    summary_out_file = join(summary_out_dir, "billing_summary.txt")
    n_workers = _get_value_from_run_name(run_name, "WORKERS")

    results = {}

    # Pull all files into a big dictionary
    for filename in listdir(result_dir):
        file_path = join(result_dir, filename)
        hostname = filename.replace(".log", "")

        results[hostname] = defaultdict(list)
        print("Reading data for host {}".format(hostname))

        # Parse data into series of values
        with open(file_path, "r") as fh:
            for line in fh:
                timestamp, metric, value = line.split(" ")

                # Convert to numbers
                timestamp = Decimal(timestamp.strip())
                value = Decimal(value.strip())

                results[hostname][metric].append((timestamp, value))

    total_net_sent = _total_diff_across_all(results, "NET_SENT_MB")
    total_cpu_user = _total_diff_across_all(results, "CPU_TIME_USER")
    total_cpu_iowait = _total_diff_across_all(results, "CPU_TIME_IOWAIT")
    total_cpu_idle = _total_diff_across_all(results, "CPU_TIME_IDLE")
    total_disk_write = _total_diff_across_all(results, "DISK_WRITE_MB")

    with open(summary_out_file, "w") as fh:
        fh.write("WORKERS  {}\n".format(n_workers))
        fh.write("NET_SENT_MB  {}\n".format(total_net_sent))
        fh.write("DISK_WRITE_MB  {}\n".format(total_disk_write))
        fh.write("CPU_USER  {}\n".format(total_cpu_user))
        fh.write("CPU_IOWAIT  {}\n".format(total_cpu_iowait))
        fh.write("CPU_IDLE  {}\n".format(total_cpu_idle))


def _total_diff_across_all(results, metric_name):
    total_val = 0
    for host, stats in results.items():
        total_val += _get_diff_metric(stats, metric_name)

    return total_val


def _get_diff_metric(host_stats, metric_name):
    metric_data = host_stats[metric_name]

    metric_data.sort(key=lambda x: x[0])
    metric_data = metric_data[-1][1] - metric_data[0][1]
    return metric_data


def _get_min_max_diff_metric(host_stats, metric_name):
    metric_data = host_stats[metric_name]

    values = [m[1] for m in metric_data]
    return max(values) - min(values)


def _get_avg_metric(host_stats, metric_name):
    metric_data = host_stats[metric_name]

    values = [m[1] for m in metric_data]
    return np.mean(values)


def _get_value_from_run_name(run_name, variable):
    parts = run_name.split("_")

    try:
        idx = parts.index(variable)
    except ValueError:
        return None

    variable_value = parts[idx + 1]
    try:
        variable_value = int(variable_value)
    except ValueError:
        pass

    return variable_value
