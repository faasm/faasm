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

        # CPU
        cpu_pct = psutil.cpu_percent(percpu=False)
        cpu_pct_per_cpu = psutil.cpu_percent(percpu=True)
        cpu_times = psutil.cpu_times(percpu=False)
        cpu_times_pct = psutil.cpu_times_percent(percpu=False)

        stats.extend([
            ("CPU_PCT", cpu_pct),
            ("CPU_PCT_IOWAIT", cpu_times_pct.iowait),
            ("CPU_PCT_IDLE", cpu_times_pct.idle),
            ("CPU_TIME_IOWAIT", cpu_times.iowait),
            ("CPU_TIME_USER", cpu_times.user),
            ("CPU_TIME_SYSTEM", cpu_times.system),
        ])

        for cpu_num, cpu_pct in enumerate(cpu_pct_per_cpu):
            stats.append(("CPU_{}_PCT".format(cpu_num), cpu_pct))

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
    parser = argparse.ArgumentParser()
    parser.add_argument('--truncate', default=False, action='store_true')
    args = parser.parse_args()

    if not exists(OUTPUT_DIR):
        makedirs(OUTPUT_DIR)

    if exists(OUTPUT_FILE):
        remove(OUTPUT_FILE)

    monitor = SystemStatsMonitor()

    monitor.initialise()

    while True:
        monitor.print_stats()
        sleep(SLEEP_INTERVAL_SECS)
