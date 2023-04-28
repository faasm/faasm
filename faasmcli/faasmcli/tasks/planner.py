from faasmcli.util.endpoints import get_planner_host_port
from faasmcli.util.http import do_post
from faasmcli.util.planner import prepare_planner_msg
from invoke import task


@task(default=True)
def reset(ctx):
    """
    Reset the planner state
    """
    host, port = get_planner_host_port()
    msg = prepare_planner_msg("RESET")

    url = "http://{}:{}".format(host, port)
    return do_post(url, msg, quiet=False, json=True)
