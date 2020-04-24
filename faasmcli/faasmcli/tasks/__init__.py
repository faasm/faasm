from invoke import Collection

# from . import ibm
from . import bare_metal
from . import bench_capacity
from . import bench_mem
from . import bench_time
from . import bench_tpt
from . import codegen
from . import compile
from . import config
from . import disas
from . import docker
from . import github
from . import call
from . import knative
from . import libs
from . import python
from . import redis
from . import prk
from . import run
from . import toolchain
from . import upload
from . import wast

# Default names
ns = Collection(
    # aws,
    # ibm,
    codegen,
    compile,
    config,
    disas,
    docker,
    github,
    knative,
    libs,
    prk,
    python,
    redis,
    run,
    toolchain,
    upload,
    wast,
)

# Custom names
ns.add_collection(ns.from_module(call), name="invoke")
ns.add_collection(ns.from_module(bare_metal), name="bm")

# Benchmarking
bench_ns = Collection("bench")
bench_ns.add_task(bench_capacity.max_threads)
bench_ns.add_task(bench_time.bench_time, name="time")
bench_ns.add_task(bench_mem.bench_mem, name="mem")
bench_ns.add_task(bench_tpt.bench_tpt, name="tpt")
bench_ns.add_task(bench_mem.pid_mem, name="pid")
bench_ns.add_task(bench_mem.container_count)
bench_ns.add_task(bench_mem.faasm_count)
bench_ns.add_task(bench_mem.spawn_containers)
bench_ns.add_task(bench_mem.kill_containers)
bench_ns.add_task(bench_mem.kill_faasm)
bench_ns.add_task(bench_mem.plot_pid_mem)
bench_ns.add_task(bench_mem.plot_proc_mem)
bench_ns.add_task(bench_mem.print_docker_mem)
ns.add_collection(bench_ns)

# Can only generate matrix data with things installed
try:
    import pyfaasm
    from . import matrix_data

    ns.add_collection(ns.from_module(matrix_data))
except:
    pass
