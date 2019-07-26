import psutil
from psutil import process_iter
import matplotlib.pyplot as plt;

plt.rcdefaults()
import numpy as np
import matplotlib.pyplot as plt


class MemTotal:
    def __init__(self):
        self.vss = 0
        self.uss = 0
        self.pss = 0
        self.rss = 0
        self.shared = 0
        self.text = 0
        self.lib = 0
        self.data = 0
        self.dirty = 0

    def get_labels(self):
        return [
            "VSS", "USS", "PSS", "RSS", "Shared", "Text", "Lib", "Data", "Dirty"
        ]

    def add_mem_info(self, mi):
        self.vss += mi.vms
        self.uss += mi.uss
        self.pss += mi.pss
        self.rss += mi.rss
        self.shared += mi.shared
        self.text += mi.text
        self.lib += mi.lib
        self.data += mi.data
        self.dirty += mi.dirty

    def get_data(self):
        return [self.vss, self.uss, self.pss, self.rss, self.shared, self.text, self.lib, self.data, self.dirty]

    def print(self):
        for label, datum in zip(self.get_labels(), self.get_data()):
            datum /= (1024 * 1024)
            print("{}={:.2f}MB".format(label, datum))

    def plot(self, exclude_vss=True):
        labels = self.get_labels()
        data = self.get_data()

        if exclude_vss:
            labels = labels[1:]
            data = data[1:]

        y_pos = np.arange(len(labels))
        data = [d / (1024 * 1024) for d in data]

        plt.bar(y_pos, data, align='center', alpha=0.5)

        plt.xticks(y_pos, labels)
        plt.ylabel("MB")
        plt.title("Memory breakdown")

        plt.show()


def get_pid_for_name(proc_part):
    procs = [p for p in process_iter(attrs=["pid", "name"]) if proc_part in p.info["name"]]

    n_procs = len(procs)
    if n_procs == 0:
        raise RuntimeError("Could not find process {}".format(proc_part))

    if n_procs > 1:
        raise RuntimeError("Found {} processes {}".format(n_procs, proc_part))

    proc = procs[0]
    return proc.pid


def get_total_memory_for_pid(pid):
    p = psutil.Process(pid=pid)
    m = MemTotal()

    _get_memory_for_process(p, m)

    print("{}\n{}".format(m.get_labels(), m.get_data()))

    return m


def _get_memory_for_process(process, m):
    mem_info = process.memory_full_info()
    print("PID {} = {}".format(process.pid, mem_info))

    m.add_mem_info(mem_info)

    # Recurse through children and add up
    for c in process.children():
        _get_memory_for_process(c, m)

    return m
