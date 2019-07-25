import psutil
from psutil import process_iter


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

    uss, pss, rss = _get_memory_for_process(p)

    print("PID {}: USS={} PSS={} RSS={}".format(pid, uss, pss, rss))

    return uss, pss, rss


def _get_memory_for_process(process):
    mem_info = process.memory_full_info()
    print("PID {}: USS {}, PSS {}, RSS {}".format(process.pid, mem_info.uss, mem_info.pss, mem_info.rss))

    # Get the different memory measures for the given process
    uss_total = mem_info.uss
    pss_total = mem_info.pss
    rss_total = mem_info.rss

    # Recurse through children and add up
    for c in process.children():
        child_uss, child_pss, child_rss = _get_memory_for_process(c)
        uss_total += child_uss
        pss_total += child_pss
        rss_total += child_rss

    return uss_total, pss_total, rss_total
