from decimal import Decimal
from math import ceil
from os import listdir
from os.path import join, exists

import matplotlib.pyplot as plt


class BillingData():
    def __init__(self, host):
        self.host = host

        self.cpu_pct = list()

        self.net_sent_mb = list()
        self.net_recv_mb = list()

        self.memory_used_mb = list()

        self.disk_read_mb = list()
        self.disk_write_mb = list()

        self.max_net = None
        self.max_mem = None
        self.max_disk = None
        self.max_cpu = 100

    def _rebase(self, data):
        return [(d[0] - data[0][0], d[1] - data[0][1]) for d in data]

    def _rebase_time(self, data):
        return [(d[0] - data[0][0], d[1]) for d in data]

    def _max_stat(self, data):
        return max([d[1] for d in data])

    def parse(self, billing_file):
        with open(billing_file, "r") as fh:
            for line in fh:
                dt_str, label, value = [l.strip() for l in line.split(" ") if l.strip()]

                entry = (Decimal(dt_str), Decimal(value))

                if label == "CPU_PCT":
                    self.cpu_pct.append(entry)
                elif label == "NET_SENT_MB":
                    self.net_sent_mb.append(entry)
                elif label == "NET_RECV_MB":
                    self.net_recv_mb.append(entry)
                elif label == "MEMORY_USED":
                    self.memory_used_mb.append(entry)
                elif label == "DISK_READ_MB":
                    self.disk_read_mb.append(entry)
                elif label == "DISK_WRITE_MB":
                    self.disk_write_mb.append(entry)

        self.cpu_pct = self._rebase_time(self.cpu_pct)
        self.memory_used_mb = self._rebase_time(self.memory_used_mb)

        self.disk_write_mb = self._rebase(self.disk_write_mb)
        self.disk_read_mb = self._rebase(self.disk_read_mb)
        self.net_recv_mb = self._rebase(self.net_recv_mb)
        self.net_sent_mb = self._rebase(self.net_sent_mb)

        self.max_disk = max(self._max_stat(self.disk_write_mb), self._max_stat(self.disk_read_mb))
        self.max_net = max(self._max_stat(self.net_sent_mb), self._max_stat(self.net_recv_mb))
        self.max_mem = self._max_stat(self.memory_used_mb)


def parse_billing_data(result_dir):
    billing_results_dir = join(result_dir, "billing", "results")
    if not exists(billing_results_dir):
        raise RuntimeError("Did not find billing results at {}".format(billing_results_dir))

    host_data = dict()
    for host_file in listdir(billing_results_dir):
        host_name = host_file.replace(".log", "")
        data = BillingData(host_name)
        data.parse(join(billing_results_dir, host_file))

        host_data[host_name] = data

    return host_data


def plot_stat(stat_in, **kwargs):
    x_data = [d[0] for d in stat_in]
    y_data = [d[1] for d in stat_in]
    plt.plot(x_data, y_data, **kwargs)


def plot_billing_data_multi(result_dir):
    host_data = parse_billing_data(result_dir)

    n_hosts = len(host_data.keys())
    n_rows = n_hosts // 2

    # Work out overall maxima
    max_net = -1000
    max_disk = -1000
    max_mem = -1000
    for data in host_data.values():
        max_net = max(max_net, data.max_net)
        max_disk = max(max_disk, data.max_disk)
        max_mem = max(max_mem, data.max_mem)
    max_net = int(ceil(max_net))
    max_disk = int(ceil(max_disk))
    max_mem = int(ceil(max_mem))

    # Rows of 8, 2 hosts per row
    idx = 0
    for host_name, data in host_data.items():
        is_even = idx % 2 == 0
        row_idx = idx // 2

        row_offset = row_idx * 8
        if not is_even:
            row_offset += 1

        # CPU
        ax = plt.subplot(n_rows, 8, row_offset + 1)
        plot_stat(data.cpu_pct, label=host_name)
        ax.set_ylim([0, 100])
        plt.legend()

        # Memory
        ax = plt.subplot(n_rows, 8, row_offset + 3)
        plot_stat(data.memory_used_mb)
        ax.set_ylim([0, max_mem])

        # Network
        ax = plt.subplot(n_rows, 8, row_offset + 5)
        plot_stat(data.net_sent_mb, label="sent")
        plot_stat(data.net_recv_mb, label="recv")
        ax.set_ylim([0, max_net])
        plt.legend()

        # Disk
        ax = plt.subplot(n_rows, 8, row_offset + 7)
        plot_stat(data.disk_read_mb, label="read")
        plot_stat(data.disk_write_mb, label="write")
        ax.set_ylim([0, max_disk])
        plt.legend()

        idx += 1

    fig = plt.gcf()

    fig.set_size_inches(25, 25)
    plt.tight_layout()
    plt.show()



def plot_billing_data(result_dir):
    host_data = parse_billing_data(result_dir)

    # CPU
    plt.subplot(2, 2, 1)
    for host, data in host_data.items():
        plot_stat(data.cpu_pct, label=host)
    plt.legend()

    # Memory
    plt.subplot(2, 2, 2)
    for host, data in host_data.items():
        plot_stat(data.cpu_pct, label=host)
    plt.legend()

    # Network
    plt.subplot(2, 2, 3)
    for host, data in host_data.items():
        plot_stat(data.net_sent_mb, label="{}_sent".format(host))
        plot_stat(data.net_recv_mb, label="{}_recv".format(host))
    plt.legend()

    # Disk
    plt.subplot(2, 2, 4)
    for host, data in host_data.items():
        plot_stat(data.disk_read_mb, label="{}_read".format(host))
        plot_stat(data.disk_write_mb, label="{}_write".format(host))
    plt.legend()

    fig = plt.gcf()

    fig.set_size_inches(25, 25)
    plt.tight_layout()
    plt.show()
