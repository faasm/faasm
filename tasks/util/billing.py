from collections import defaultdict
from decimal import Decimal
from os import listdir
from os.path import join
from subprocess import call

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


def parse_billing(output_dir):
    results = {}

    # Pull all files into a big dictionary
    for filename in listdir(output_dir):
        file_path = join(output_dir, filename)
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

    # For each host, work out the billing
    for host, stats in results.items():
        print("\n------ {} ------".format(host))
        print("CPU count: {}".format(stats["CPU_COUNT"][0][1]))
        print("CPU freq: {}".format(stats["CPU_FREQ"][0][1]))
        print("CPU time user: {}".format(_get_diff_metric(stats, "CPU_TIME_USER")))
        print("CPU time iowait: {}".format(_get_diff_metric(stats, "CPU_TIME_IOWAIT")))
        print("CPU time system: {}".format(_get_diff_metric(stats, "CPU_TIME_SYSTEM")))

        print("NET sent: {}".format(_get_diff_metric(stats, "NET_SENT_MB")))
        print("NET recv: {}".format(_get_diff_metric(stats, "NET_RECV_MB")))

        print("DISK read: {}".format(_get_diff_metric(stats, "DISK_READ_MB")))
        print("DISK write: {}".format(_get_diff_metric(stats, "DISK_WRITE_MB")))

        print("MEM used: {}".format(_get_min_max_diff_metric(stats, "MEMORY_USED")))
        print("MEM active: {}".format(_get_min_max_diff_metric(stats, "MEMORY_ACTIVE")))


def _get_diff_metric(host_stats, metric_name):
    metric_data = host_stats[metric_name]

    metric_data.sort(key=lambda x: x[0])
    metric_data = metric_data[-1][1] - metric_data[0][1]
    return metric_data


def _get_min_max_diff_metric(host_stats, metric_name):
    metric_data = host_stats[metric_name]

    values = [m[1] for m in metric_data]
    return max(values) - min(values)
