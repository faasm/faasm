# Benchmarks

## Runtimes

For efficiency measurements we want to assess the time taken and resources consumed by several different runtimes. To do this we need to measure the time taken, CPU cycles, peak memory and disk I/O for running a simple function (with a cold start).

Measuring the CPU cycles and time taken can be done with `perf` and `time` respectively (in separate runs). To measure memory footprint we take the footprint of the relevant process and all its descendants. To do this for Docker we need to take the footprint of the docker daemon and all of its child processes, thus capturing the whole cost of running Docker on the host.

Measurements are taken for a vanilla Docker container (Alpine) and Faasm, both running a noop function. The Docker container will run the unmodified native binary and Faasm will run its generated shared object from the WebAssembly for the function.

To amortize any start-up time and underlying system resources we run each for multiple iterations and varying numbers of workers (containers for Docker, threads for Faasm).

### Set up

To run on a remote machine, you need to set up an inventory file at `ansible/inventory/benchmark.yml`, e.g.

```
[all]
my.hostname.blah
```

You can then set up the machine with:

```
./bin/provision_bench_host.sh
```

You'll need to restart the host once Ansible has finished.

Once the host is fully set up, you can SSH onto it and run:

```
cd /usr/local/code/faasm
./bin/set_up_benchmarks.sh
```

For Docker you need to build the `faasm/noop` image which can be done with:

```
inv docker-build-noop
```

### Memory

What we mean by "footprint" is worth clarifying. The resident set size (RSS) is often used, however this double counts memory shared between two processes. A more appropriate measure is the proportional set size (PSS), which spreads the "cost" of any shared memory between those sharing it. 

For a multi-threaded single process like Faasm these two values are (almost) the same. For Docker though, it makes a big difference depending on the containers in question.

Docker containers from the same image will share the same layered filesystem. This means binaries and shared libraries are mapped into shared memory shared between the containers. If we have two containers from the same image, their PSS will be much lower than the sum of their RSS. If we have two containers from totally different images, the sum of their PSS will be much closer to the sum of their RSS.

In a multi-tenant environment the containers being run will likely be heterogeneous to some extent, but not completely. In our experiments we use container from exacly the same image, thus showing a big difference between PSS and RSS. The real world impact would be somewhere between these two.  

The Faasm memory footprint is quite a lot larger than standard threads running the same native code. Aside from the base cost of loading the libraries for the runtime itself, the incremental cost of adding more workers comes from the extra heap space. This heap space is taken up by the LLVM objects created by the JITing, WAVM objects holding the module definition, memories, tables and functions, plus the actual linear memory of the functions themselves.

### Running

The timing and CPU measurements can be taken by running:

```
inv bench-time
```

Results are written to `~/faasm/results/runtime-bench-time.csv`.

The memory measurements require access to details of the Docker daemon, hence need to be run as root:

```
# Remove all Docker containers in case
docker ps -aq | xargs docker rm

# Run the benchmark as root
sudo su
source workon.sh
inv bench-mem
```

Results are written to `/root/faasm/results/runtime-bench-mem.csv` (assuming your root home is `/root`).

### Other runtimes

It would be nice to benchmark these similar overheads against Firecracker, however I haven't been able to script it
up yet. It can be set up by running:

```
cd ansible
ansible-playbook firecracker.yml --ask-become-pass
```

Lucet would also be interesting but it doesn't currently provide a
[full isolation environment](https://github.com/fastly/lucet/security/policy), therefore is not really comparable.

## Capacity

Capacity measurements aim to work out the maximum number of concurrent workers we can sustain on a given box for both Faasm and Docker.

### Docker

Spawning lots of Docker containers at once can lead to big memory/ CPU spikes, so we need to build up the numbers slowly. This can be done with the task:

```
# Spawn 100 containers
inv spawn-docker-containers 100
```

We can only have 1023 Docker containers on a single Docker network (due to the limit on virtual bridges), so we can either run them all on the `host` network, or create a couple of bigger networks, e.g.

```
# Create 2 networks
docker network create bench-1
docker network create bench-2

# Spawn 600 on each
inv spawn-docker-containers 600 --network=bench-1
inv spawn-docker-containers 600 --network=bench-2

# Check resource levels

# Finish
inv kill-containers
```

Docker may also be limited by the `TasksMax` parameter in `/lib/systemd/system/docker.service` (or equivalent). This can be set to `infinity`.

By keeping a close eye on the memory usage on the box you should be able to push the number of containers up to about 1700 with 16GiB of RAM.

### Faasm

_System Limits_

System limits will normally limit the capacity for Faasm workers rather than the application itself.

The most likely will be limits on the max number of threads, which you can test using:

```
# Note, ulimit will fail if your hard limit is too low
ulimit -u 120000

inv max-threads
```

This will show both the system max and what you can currently reach. If this is low you can do the following:

- Switch off any `systemd` accounting (add `DefaultTasksAccounting=no` in `/etc/systemd/system.conf`)
- Set `UserTasksMax=infinity` in `/etc/systemd/logind.conf`
- Bump up system limits by setting `kernel.pid_max=150000` and `vm.max_map_count=1000000` via `sysctl`
- Restart
- In the shell you're running the test, raise the `nproc` and `stack` limits with `ulimit`
- You may need to edit `/etc/security/limits.conf` to raise hard limits if you can't raise the `ulimit` values high enough

_Running the Faasm capcity experiments_

Because WAVM allocates so much virtual memory to each module we need to divide the workers across a couple of processes (to avoid the hard 128TiB per-process virtual memory limit).

To keep the functions hanging around we can use the `demo/lock` function which sits around waiting for a lock file at `/usr/local/faasm/runtime_root/tmp/demo.lock`. It'll drop out once this has been removed.

To spawn a large number of workers we can do the following:

```
source workon.sh

# Spawn lots of workers in one terminal (will wait until killed)
./bin/spawn_faasm.sh 65000

# Print thread count
inv faasm-count

# Check resources

# Kill and check
pkill -f bench_mem
inv faasm-count
```

If there is enough memory on the box, both Faasm and Docker will eventually be limited by the     max threads in the system (`cat /proc/sys/kernel/threads-max`).

### Redis

Although it shouldn't be involved in the capactiy benchmark, Redis has a default limit of 10000 clients. To raise this you can use the client or edit `/etc/redis/redis.conf`:

```
# Redis clients
redis-cli
config set maxclients 50000
```

## Throughput

To asses the throughput capacity of both Faasm and Docker, we want to execute increasing numbers of functions per second. With Faasm this means executing a noop function over and over, and Docker executing a noop inside a minimal container. We are not including the removal of the Docker container in the Docker numbers (as this would be done periodically in the background).

To run the throughput benchmark you can run:

```
source workon.sh
inv bench-tpt
```

The results will be output at `~/faasm/results/runtime-bench-tpt.csv`.

## Polybench/C

To test pure computation against the native environment we can use the
[Polybench/C benchmark](http://web.cse.ohio-state.edu/~pouchet.2/software/polybench/).

The code is checked into this repository and can be compiled to wasm and uploaded as follows:

```
# Compile to wasm
inv compile --user=polybench --clean

# Upload (must have an upload server running)
inv upload-all
```

We can compile the same functions natively as follows:

```
./bin/build_polybench_native.sh
```

The `poly_bench` executable will then run a comparison of the wasm and native versions. This must
be invoked with your desired number of iterations for native and wasm respectively, e.g.

```
poly_bench all 5 5
```

Results are currently output to `/tmp/polybench.csv`.

_Note - we had to leave out the BLAS benchmarks as BLAS is not supported in Faasm_.

## Python

To benchmark CPython execution we use the [Python Performance Benchmark Suite](https://github.com/python/performance).

All python code runs in the same function which can be set up according to the python docs in this repo.

The set of benchmarks can be run with the `py_bench` executable. You can specify a specific benchmark or a single
benchmark, along with the number of wasm and native iterations, e.g. `py_bench all 100 100` or
`py_bench bench_float.py 200 200`. This will output to `/tmp/pybench.csv`.

Each benchmark requires porting the required dependencies, so some were unfeasible and other were too much work:

- `chameleon` - too many deps
- `django_template` - pulls in too many dependencies
- `hg_startup` - runs a shell command
- `html5lib` - dependencies (might be fine)
- `pathlib` - requires more access to the filesystem that we support
- `python_startup` - runs a shell command
- `regex_compile` - needs to import several other local modules (should be possible, just fiddly)
- SQL-related - SQLAlchemy not worth porting for now. SQLite also not supported but could be
- `sympy` - sympy module not yet ported but could be
- `tornado` - Tornado not ported (and don't plan to)


## Profiling

### Profiling Polybench/C

To profile the native version of the code, you need to run `./bin/build_polybench_native.sh Debug`

Then you can directly run the native binary:

```
perf record -k 1 ./func/build_native/polybench/poly_ludcmp
mv perf.data perf.data.native
perf report -i perf.data.native
```

Provided you have set up the wasm profiling set-up as described in the profiling docs, you can do something
similar:

```
perf record -k 1 poly_bench poly_ludcmp 0 5
perf inject -i perf.data -j -o perf.data.wasm
perf report -i perf.data.wasm
```

Note that for wasm code the output of the perf reports will be funciton names like `functionDef123`.
To generate the mapping of these names to the actual functions you can run the `func_sym` executable
e.g.

```
func_sym polybench 3mm
```
