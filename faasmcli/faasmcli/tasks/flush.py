from faasmcli.util.endpoints import get_invoke_host_port, get_planner_host_port
from faasmcli.util.http import do_post
from faasmcli.util.planner import PLANNER_MESSAGE_TYPE
from invoke import task

FAABRIC_MSG_TYPE_FLUSH = 3


@task(default=True)
def all(ctx):
    """
    Flush functions, state and shared files from all workers
    """
    host, port = get_invoke_host_port()
    msg = {
        "type": FAABRIC_MSG_TYPE_FLUSH,
    }

    url = "http://{}:{}".format(host, port)
    return do_post(url, msg, quiet=False, json=True)


@task
def hosts(ctx):
    """
    Flush the set of available hosts
    """
    host, port = get_planner_host_port()
    msg = {"type": PLANNER_MESSAGE_TYPE["FLUSH_HOSTS"]}

    url = "http://{}:{}".format(host, port)
    return do_post(url, msg, quiet=False, json=True)
