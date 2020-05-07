from invoke import task

from faasmcli.util.call import invoke_impl, status_call_impl, flush_call_impl
from faasmcli.util.endpoints import get_invoke_host_port

import time
import redis
import subprocess
from subprocess import call, run

micro_time = lambda: int(time.perf_counter() * 1000_000)
def time_func_microseconds(func):
    start =  micro_time()
    func()
    return micro_time() - start

@task
def multi_cr(ctx, debug=False, num_times=200, num_threads=1):
    output_file = f"/usr/local/code/faasm/wasm/omp/multi_cr/bench_all.csv"
    modes = {
        "native": 0,
        "wasm": -1,
    }

    func = "multi_cr"

    threads = [1] + list(range(2, 25, 2))

    r = redis.Redis(host="localhost")
    rkey = f"{func}_fork_times"
    rkey1 = f"{func}_local_fork_times"
    r.delete(rkey)
    # rindex = 0

    with open(output_file, "w") as csv:
        csv.write("numThreads,type,microseconds\n")
        for num_threads in threads:
            cmd = f"{num_threads} {num_times} 0"
            print(f"NATIVE: running omp/multi_cr-- {cmd}")
            # t_native = run(["/usr/local/code/faasm/ninja-build/bin/multi_cr", f"{num_threads}", "1", "0"], stdout=subprocess.PIPE).stdout.decode('utf-8')
            t_native = run(["/usr/local/code/faasm/ninja-build/bin/multi_cr", f"{num_threads}", f"{num_times}", "0"], stdout=subprocess.PIPE).stdout.decode('utf-8')
            csv.write(t_native)

            # WasmMP
            cmd = f"{num_threads} {num_times} 0"
            print(f"WASM running omp/multi_cr-- {cmd}")
            invoke_impl("omp", "multi_cr_local", knative=True, cmdline=cmd)
            if r.llen(rkey1) != num_times:
                print("Failed to run wasm")
                exit(1)
            t_wasm = list(map(int, r.lrange(rkey1, 0, num_times)))
            for t in t_wasm:
                wasm_line = f"{num_threads},WasmMP,{t}\n"
                csv.write(wasm_line)
            r.flushall()

            # faasmp
            cmd = f"{num_threads} {num_times} -1"
            print(f"FAASM running omp/multi_cr-- {cmd}")
            invoke_impl("omp", "multi_cr", knative=True, cmdline=cmd)
            if r.llen(rkey) != num_times:
                print("Failed to run faasm")
                exit(1)
            t_wasm = list(map(int, r.lrange(rkey, 0, num_times)))
            r.delete(rkey)
            for t in t_wasm:
                wasm_line = f"{num_threads},FaasMP,{t}\n"
                csv.write(wasm_line)

            r.flushall()



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
