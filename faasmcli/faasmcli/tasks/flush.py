from faasmcli.util.endpoints import get_planner_host_port
from faasmcli.util.http import do_post
from faasmcli.util.planner import PLANNER_MESSAGE_TYPE
from invoke import task

FAABRIC_MSG_TYPE_FLUSH = 3


@task(default=True)
def all(ctx):
    """
    Flush functions, state and shared files from all workers
    """
    workers(ctx)

    # Flush hosts last, as we need the host list to propagate the other flush
    # requests to the workers
    hosts(ctx)


@task
def hosts(ctx):
    """
    Flush the set of available hosts
    """
    host, port = get_planner_host_port()
    msg = {"type": PLANNER_MESSAGE_TYPE["FLUSH_HOSTS"]}

    url = "http://{}:{}".format(host, port)
    return do_post(url, msg, quiet=False, json=True)


@task
def workers(ctx):
    """
    Flush cached files and machine code from workers
    """
    host, port = get_planner_host_port()
    msg = {"type": PLANNER_MESSAGE_TYPE["FLUSH_EXECUTORS"]}

    url = "http://{}:{}".format(host, port)
    return do_post(url, msg, quiet=False, json=True)
