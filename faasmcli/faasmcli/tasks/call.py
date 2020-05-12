from invoke import task

from faasmcli.util.call import invoke_impl, status_call_impl, flush_call_impl
from faasmcli.util.endpoints import get_invoke_host_port

import redis
import time

@task(default=True)
def invoke(ctx, user, func,
           host=None,
           port=None,
           input=None,
           py=False,
           async=False,
           knative=True,
           native=False,
           ibm=False,
           poll=False,
           cmdline=None,
           debug=False,
           ):
    """
    Invoke a function
    """
    invoke_impl(user, func, host=host, port=port, input=input, py=py, async=async,
                knative=knative, native=native, ibm=ibm, poll=poll, cmdline=cmdline, debug=debug)

@task
def multi_cr(ctx, debug=False, num_times=6):
    backoff = lambda x: min(max(1, x // 2),  num_times)
    output_file = f"/usr/local/code/faasm/wasm/omp/multi_pi/bench_state.csv"
    func = "multi_pi"

    sizes = {
        "Small": 20_000_000,
        "Huge": 20_000_000_000,
    }

    threads = list(range(2, 41, 2))

    r = redis.Redis(host="localhost")
    rkey = f"{func}_fork_times"
    r.delete(rkey)
    rindex = 0

    with open(output_file, "w") as csv:
        csv.write("numThreads,type,microseconds\n")
        for iter_name, iter_size in sizes.items():
            for num_threads in threads:
                for _ in range(backoff(num_threads)):
                    cmd = f"{num_threads} {iter_size} -1"
                    print(f"WASM running omp/multi_cr-- {cmd}")
                    invoke_impl("omp", func, knative=True, cmdline=cmd)
                    t_wasm = int(r.lindex(rkey, rindex))
                    wasm_line = f"{num_threads},{iter_name},{t_wasm}\n"
                    csv.write(wasm_line)
                    rindex += 1
                r.flushall()
                rindex = 0


@task
def status(ctx, call_id, host=None, port=None):
    """
    Get the status of an async function call
    """
    k8s_host, k8s_port = get_invoke_host_port()
    host = host if host else k8s_host
    port = port if port else k8s_port

    status_call_impl(call_id, host, port)


@task
def flush(ctx):
    """
    Flush workers
    """
    host, port = get_invoke_host_port()
    host = host if host else "127.0.0.1"
    port = port if port else 8080

    flush_call_impl(host, port)
