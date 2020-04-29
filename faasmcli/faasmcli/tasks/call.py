from invoke import task

from faasmcli.util.call import invoke_impl, status_call_impl, flush_call_impl
from faasmcli.util.endpoints import get_invoke_host_port

import time
from subprocess import call
import redis

current_milli_time = lambda: int(round(time.time() * 1000))

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
def multi_pi(ctx, number_times=5):
    output_file = "/usr/local/code/faasm/multi_pi.csv"
    # * 2 in this experiment since starting from `- iterations` instead of 0
    sizes = {
        "Tiny": 100000,
        "Small": 10000000,
        "Big": 100000000,
        "Huge": 600000000,
    }
    threads = [1] + list(range(2, 25, 2))

    r = redis.Redis(host="koala10")
    times_key = "multi_pi_times"
    r.delete(times_key)
    num_times = 0
    for _ in range(number_times):
        for iter_size in sizes.values():
            for num_threads in threads:
                cmd = f"{num_threads} {iter_size}"
                print(f"running omp/multi_pi -- {cmd}")
                invoke_impl("omp", "multi_pi", knative=True, cmdline=cmd)
                # allow for async inboke
                while r.llen(times_key) == num_times:
                    print("Waiting for function to finish")
                    time.sleep(1.5)
                num_times += 1

    times = list(map(int, r.lrange(times_key, 0, num_times)))
    assert(len(times) == num_times)
    idx = 0
    with open(output_file, "w") as csv:
        csv.write("iterations,numThreads,type,milliseconds\n")
        for _ in range(number_times):
            for iter_name in sizes.keys():
                for num_threads in threads:
                    result = f"{iter_name},{num_threads},distributed,{times[idx]}\n"
                    idx += 1
                    csv.write(result)


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
