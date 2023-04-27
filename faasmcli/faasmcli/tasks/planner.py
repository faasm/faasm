from faasmcli.util.endpoints import get_planner_host_port
from faasmcli.util.http import do_post
from faasmcli.util.planner import PLANNER_MESSAGE_TYPE
from invoke import task


@task(default=True)
def reset(ctx):
    """
    Reset the planner state
    """
    host, port = get_planner_host_port()
    msg = {"type": PLANNER_MESSAGE_TYPE["RESET"]}

    url = "http://{}:{}".format(host, port)
    return do_post(url, msg, quiet=False, json=True)
