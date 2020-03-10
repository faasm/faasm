from invoke import Collection

# import tasks.aws
# import tasks.ibm
import tasks.bare_metal
import tasks.bench_capacity
import tasks.bench_mem
import tasks.bench_time
import tasks.bench_tpt
import tasks.codegen
import tasks.compile
import tasks.config
import tasks.data
import tasks.docker
import tasks.experiments
import tasks.genomics
import tasks.github
import tasks.invoke
import tasks.knative
import tasks.libs
import tasks.python
import tasks.redis
import tasks.prk
import tasks.run
import tasks.toolchain
import tasks.upload
import tasks.wast

# Default names
ns = Collection(
    # tasks.aws,
    # tasks.ibm,
    tasks.codegen,
    tasks.compile,
    tasks.config,
    tasks.data,
    tasks.docker,
    tasks.experiments,
    tasks.genomics,
    tasks.github,
    tasks.invoke,
    tasks.knative,
    tasks.libs,
    tasks.prk,
    tasks.python,
    tasks.redis,
    tasks.run,
    tasks.toolchain,
    tasks.upload,
    tasks.wast,
)

# Custom names
ns.add_collection(ns.from_module(tasks.bare_metal), name="bm")

# Benchmarking
bench_ns = Collection("bench")
bench_ns.add_task(tasks.bench_capacity.max_threads)
bench_ns.add_task(tasks.bench_time.bench_time, name="time")
bench_ns.add_task(tasks.bench_mem.bench_mem, name="mem")
bench_ns.add_task(tasks.bench_tpt.bench_tpt, name="tpt")
bench_ns.add_task(tasks.bench_mem.pid_mem, name="pid")
bench_ns.add_task(tasks.bench_mem.container_count)
bench_ns.add_task(tasks.bench_mem.faasm_count)
bench_ns.add_task(tasks.bench_mem.spawn_containers)
bench_ns.add_task(tasks.bench_mem.kill_containers)
bench_ns.add_task(tasks.bench_mem.kill_faasm)
bench_ns.add_task(tasks.bench_mem.plot_pid_mem)
bench_ns.add_task(tasks.bench_mem.plot_proc_mem)
bench_ns.add_task(tasks.bench_mem.print_docker_mem)
ns.add_collection(bench_ns)

# Can only generate matrix data with things installed
try:
    import pyfaasm
    import tasks.matrix_data

    ns.add_collection(ns.from_module(tasks.matrix_data))
except:
    pass
