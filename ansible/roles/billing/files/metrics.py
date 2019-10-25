import argparse
import time
from os import remove, makedirs
from os.path import exists, join
from time import sleep

import psutil

OUTPUT_DIR = "/tmp/billing/"
OUTPUT_FILE = join(OUTPUT_DIR, "metrics.log")
ONE_MB = 1024.0 * 1024.0
SLEEP_INTERVAL_SECS = 2


class SystemStatsMonitor(object):

    def initialise(self):
        # cpu_percent will return zeroes to start with, so we want to make an initial call
        # https://psutil.readthedocs.io/en/latest/#psutil.cpu_percent
        psutil.cpu_percent(percpu=False, interval=None)
        psutil.cpu_percent(percpu=True, interval=None)

        # Log one-off measurements
        cpu_freq = psutil.cpu_freq()
        stats = [
            ("CPU_COUNT", psutil.cpu_count()),
            ("CPU_FREQ", cpu_freq.max)
        ]

        self._log_stats(stats)

    def print_stats(self):
        stats = []

        # MEMORY
        vmem = psutil.virtual_memory()
        swap = psutil.swap_memory()

        stats.extend([
            ("MEMORY_USED", vmem.used / ONE_MB),
            ("MEMORY_ACTIVE", vmem.active / ONE_MB),
            ("MEMORY_USED_PCT", vmem.percent),
            ("MEMORY_AVAILABLE", vmem.available / ONE_MB),
            ("SWAP_USED", swap.used / ONE_MB),
        ])

        # DISK
        disk = psutil.disk_io_counters(perdisk=False)
        stats.extend([
            ("DISK_READ_MB", disk.read_bytes / ONE_MB),
            ("DISK_WRITE_MB", disk.write_bytes / ONE_MB),
        ])

        # CPU percent
        cpu_pct = psutil.cpu_percent(percpu=False)
        cpu_pct_per_cpu = psutil.cpu_percent(percpu=True)
        stats.append(("CPU_PCT", cpu_pct))
        for cpu_idx, p in enumerate(cpu_pct_per_cpu):
            stats.append(("CPU_{}_PCT".format(cpu_idx), p))

        # CPU times
        cpu_times = psutil.cpu_times(percpu=False)
        cpu_times_per_cpu = psutil.cpu_times(percpu=True)
        stats.extend([
            ("CPU_TIME_IDLE", cpu_times.idle),
            ("CPU_TIME_IOWAIT", cpu_times.iowait),
            ("CPU_TIME_USER", cpu_times.user),
            ("CPU_TIME_SYSTEM", cpu_times.system),
        ])
        for cpu_idx, t in enumerate(cpu_times_per_cpu):
            stats.extend([
                ("CPU_{}_TIME_IDLE".format(cpu_idx), t.idle),
                ("CPU_{}_TIME_IOWAIT".format(cpu_idx), t.iowait),
                ("CPU_{}_TIME_USER".format(cpu_idx), t.user),
                ("CPU_{}_TIME_SYSTEM".format(cpu_idx), t.system),
            ])

        # CPU percent
        cpu_times_pct = psutil.cpu_times_percent(percpu=False)
        cpu_times_pct_per_cpu = psutil.cpu_times_percent(percpu=True)
        stats.extend([
            ("CPU_PCT_IDLE", cpu_times_pct.idle),
            ("CPU_PCT_IOWAIT", cpu_times_pct.iowait),
            ("CPU_PCT_USER", cpu_times_pct.user),
            ("CPU_PCT_SYSTEM", cpu_times_pct.system),
        ])
        for cpu_idx, p in enumerate(cpu_times_pct_per_cpu):
            stats.extend([
                ("CPU_{}_PCT_IDLE".format(cpu_idx), p.idle),
                ("CPU_{}_PCT_IOWAIT".format(cpu_idx), p.iowait),
                ("CPU_{}_PCT_USER".format(cpu_idx), p.user),
                ("CPU_{}_PCT_SYSTEM".format(cpu_idx), p.system),
            ])

        # NETWORK
        network = psutil.net_io_counters(pernic=False)

        stats.extend([
            ("NET_SENT_MB", network.bytes_sent / ONE_MB),
            ("NET_RECV_MB", network.bytes_recv / ONE_MB),
        ])

        self._log_stats(stats)

    def _log_stats(self, stats):
        # Do the logging
        dt_now = time.time()
        with open(OUTPUT_FILE, "a") as fh:
            for stat_name, stat_value in stats:
                fh.write("{:.2f} {} {}\n".format(dt_now, stat_name, stat_value))


if __name__ == "__main__":
    if not exists(OUTPUT_DIR):
        makedirs(OUTPUT_DIR)

    if exists(OUTPUT_FILE):
        remove(OUTPUT_FILE)

    monitor = SystemStatsMonitor()

    monitor.initialise()

    while True:
        monitor.print_stats()
        sleep(SLEEP_INTERVAL_SECS)
