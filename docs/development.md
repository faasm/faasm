# Faasm Development

The Faasm project is roughly separated into stuff that generates WebAssembly and
stuff that executes WebAssembly.

The most important repos

- [faasm/cpp](https://github.com/faasm/cpp) - tools for building C/C++ for use
  in Faasm.
- [faasm/python](https://github.com/faasm/python) - tools for building CPython
  and Python functions for use in Faasm.
- [faasm/faasm](https://github.com/faasm/faasm) - the Faasm runtime (independent
  of specific languages used to compile the WebAssembly)
- [faasm/faabric](https://github.com/faasm/faabric) - serverless scheduling,
  messaging and state (independent of WebAssembly).

## Initial Set-up

The Faasm development environment is containerised, as defined in the [Docker
compose config](docker-compose.yml).

We mount local checkouts of all the code into these containers, so first you'll
need to update all the submodules (may take a while):

```bash
git submodule update --init --recursive
```

To tie together the projects in the local dev environment, we mount
`dev/faasm-local` into `/usr/local/faasm` in the various containers, which you
can initialise with:

```bash
./bin/refresh_local.sh
```

If you want to nuke your existing `dev/faasm-local` in the process:

```bash
./bin/refresh_local.sh -d
```

It may also be useful to run Python scripts outside the containerised
environments, for which you can set up a suitable Python virtual envrionment
with:

```bash
./bin/create_venv.sh
```

## Use

Once you've set up the repo, you can start the CLI for whichever project you
want to work on:

```bash
# C++ applications
./bin/cli.sh cpp

# Python applications
./bin/cli.sh python

# Faasm
./bin/cli.sh faasm

# Faabric
./bin/cli.sh faabric
```

## Tests

To check everything works, you can run the tests as follows (note which
container you need to be in for each step):

```bash
# --- CPP CLI ---
# Build CPP functions and lib required for the tests
inv func.local
inv libfake

# --- Python CLI ---
# Build Python wrapper function
inv func

# Upload the Python functions
inv func.upload-all --local

# --- Faasm CLI ---
# Build the development tools
inv dev.tools

# Run codegen (this may take a while the first time it's run)
inv codegen.local
inv python.codegen

# Set up cgroup
./bin/cgroup.sh

# If running with sanitisers, set their options
export ASAN_OPTIONS="halt_on_error=1"
export LSAN_OPTIONS="suppressions=/usr/local/code/faasm/leak-sanitizer-ignorelist.txt"
export TSAN_OPTIONS="halt_on_error=1:suppressions=/usr/local/code/faasm/thread-sanitizer-ignorelist.txt:history_size=7:second_deadlock_stack=1"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

# Run the tests
tests
```

## Tooling - editors, IDEs etc.

You can use custom containers that inherit from the existing CLI images if you
want to add text editors etc.

Before running the `./bin/cli.sh` script, you need to set one or more of the
following environment variables:

```bash
# Faasm
FAASM_CLI_IMAGE

# Faabric
FAABRIC_CLI_IMAGE

# CPP
CPP_CLI_IMAGE

# Python
PYTHON_CLI_IMAGE
```

The defaults for these are set in [`.env`](../.env).

## Testing

We use [Catch2](https://github.com/catchorg/Catch2) for testing and your life
will be much easier if you're familiar with their [command line
docs](https://github.com/catchorg/Catch2/blob/v2.x/docs/command-line.md).  This
means you can do things like:

```bash
# Run all the MPI tests
tests "[mpi]"

# Run a specific test
tests "Test some feature"
```

## Building outside of the container

If you want to build projects outside of the recommended containers, or just
run some of the CLI tasks, you'll need to install the required packages on your
local machine.

This is not the recommended approach to developing Faasm, so it's not scripted,
but you can work out what's required by looking at the following Dockerfiles:

- [`faasm/grpc-root`](https://github.com/faasm/faabric/blob/master/docker/grpc-root.dockerfile)
- [`faasm/cpp-root`](../docker/cpp-root.dockerfile)

Most things can be done with `apt`, but the difficult bits might be:

- LLVM and Clang
- gRPC
- Up-to-date CMake

You will also need to set up the Python environment:

```bash
# Set up the venv
./bin/create_venv.sh

# Activate the Faasm virtualenv
source bin/workon.sh

# Check things work
inv -l

cd clients/cpp
inv -l

cd ../python
inv -l
```

If you've been running the containerised development environment too, it's
highly likely that the permissions will be messed up and cause cryptic error
messages. To be safe, reset the permissions on everything:

```bash
sudo chown -R ${SUDO_USER}:${SUDO_USER} .
```

Then you can try building one of the executables:

```bash
# Check environment is correct and build directories look sane
env | grep FAASM

# Run cmake and build
inv dev.cmake
inv dev.cc func_runner

# Check which binary is on the path
which func_runner

# Run it
func_runner demo hello
```

## Code style

Code style is checked as part of the CI build and uses the following
configuration:

- **C/C++** - [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html)
  with the [Mozilla C++
  style](https://firefox-source-docs.mozilla.org/code-quality/coding-style/coding_style_cpp.html)
- **Python** - [Black](https://github.com/psf/black) with an 80 char line limit
- **Markdown** - wrap to 80 characters where possible (not enforced)
- **CMake** - TBD
- **Bash** - TBD

## Networking

If you want to switch on network isolation you need to set up network
namespaces. To do this we need to ensure consistent interface naming (`eth0` for
main public interface). If your public interface is already called `eth0` then
you can skip this step.

- Edit `/etc/default/grub` and add `net.ifnames=0 biosdevname=0` to
  `GRUB_CMDLINE_LINUX_DEFAULT`
- Run `sudo update-grub`
- Restart the machine

Then, this script will set up the namespaces. From outside the container run:

```bash
source ./bin/workon.sh
inv network.create-ns 100
```

## Cgroups

To use cgroup isolation, you'll need to run:

```bash
sudo ./bin/cgroup.sh
```

## Running a local development cluster

To start the local development cluster, you can run:

```bash
./deploy/local/dev_cluster.sh
```

### Making changes in your local cluster

Assuming you've changed something related to the `pool_runner` target (which is
executed by the `worker` container), you can pick up the changes with:

```bash
# Start the Faasm CLI
./bin/cli.sh faasm

# Rebuild the pool_runner target
inv dev.cc pool_runner
```

From a different terminal, restart the worker and check the logs:

```bash
docker-compose restart worker

docker-compose logs -f
```

### Running distributed tests locally

The distributed tests check the interactions of the Faasm workers, file storage,
redis and upload containers.

First of all, you need to make sure everything else is shut down to avoid
interfering with the tests:

```bash
docker-compose down
```

Start the distributed tests server:

```bash
./deploy/dist-test/dev_server.sh
```

Then you need to make sure all the functions are up to date:

```bash
./deploy/dist-test/upload.sh
```

Build and run the tests:

```bash
# Enter the Faasm CLI container
./bin/cli.sh faasm

# Build and run the tests
inv dev.cc dist_tests
dist_tests
```

To rebuild the server (inside the Faasm CLI container):

```bash
inv dev.cc dist_test_server
```

Then from outside the container, you can restart the server:

```bash
./deploy/dist-test/dev_server.sh restart
```

### Replicating CI

To run the distributed tests as if in CI:

```bash
# Clear up
docker-compose down

# Set up
./deploy/dist-test/build.sh
./deploy/dist-test/upload.sh

# Run once through
./deploy/dist-test/run.sh
```
