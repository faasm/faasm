from psutil import process_iter


def _get_pids_for_name(proc_part):
    procs = [p for p in process_iter(attrs=["pid", "name"]) if proc_part in p.info["name"]]
    return procs


def get_docker_parent_pids():
    """
    To get the parent PID of all Docker containers on a system, we need to find the root process.
    This will be containerd on some systems, dockerd on others
    """

    pids = []

    # Check if containerd is present
    containerd_name = "containerd"
    containerd_options = _get_pids_for_name(containerd_name)

    if len(containerd_options) == 1:
        pids.append(containerd_options[0].pid)

    elif len(containerd_options) > 1:
        raise RuntimeError("Found more than one process matching {}".format(containerd_name))

    # Get dockerd (which should always be running)
    dockerd_pid = get_pid_for_name("dockerd")
    pids.append(dockerd_pid)

    return pids


def get_pid_for_name(proc_part):
    procs = _get_pids_for_name(proc_part)
    n_procs = len(procs)
    if n_procs == 0:
        raise RuntimeError("Could not find process {}".format(proc_part))

    if n_procs > 1:
        raise RuntimeError("Found {} processes {}".format(n_procs, proc_part))

    proc = procs[0]
    return proc.pid
