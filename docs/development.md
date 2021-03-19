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

The Faasm development environment is containerised, as defined in the [dev
Docker compose config](docker-compose-dev.yml).

We mount local checkouts of all the code into these containers, so first you'll
need to update all the submodules (may take a while):

```
git submodule update --init --recursive
```

To tie together the projects in the local dev environment, we mount
`dev/faasm-local` into `/usr/local/faasm` in the various containers, which you
can initialise with:

```
./bin/refresh_local.sh
```

It may also be useful to run Python scripts outside the containerised 
environments, for which you can set up a suitable Python virtual envrionment 
with:

```
./bin/create_venv.sh
```

## Use

Once you've set up the repo, you can start the CLI for whichever project you 
want to work on:

```
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
inv func.uploadpy

# --- Faasm CLI ---
# Build the development tools
inv dev.tools

# Run codegen (this may take a while the first time it's run)
inv codegen.local
inv python.codegen

# Set up cgroup
./bin/cgroup.sh

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

## Testing

We use [Catch2](https://github.com/catchorg/Catch2) for testing and your life 
will be much easier if you're familiar with their [command line
docs](https://github.com/catchorg/Catch2/blob/v2.x/docs/command-line.md).  This
means you can do things like:

```
# Run all the MPI tests
tests "[mpi]"

# Run a specific test
tests "Test some feature"
```

## Building outside of the container

If you want to build projects outside of the recommended containers, or just
run some of the CLI tasks, you can take a look at the [CLI
Dockerfile](../docker/cli.dockerfile) to see what's required:

To run the CLI, you should just need to do:

```bash
# Set up the venv
./bin/create_venv.sh

# Activate the Faasm virtualenv
source bin/workon.sh

# Check things work
inv -l
```

## Troubleshooting CI

If the CI build fails and you can't work out why, you can replicate the test
environment locally.

First, make sure the environment variables and Docker image in 
[`docker/docker-compose-ci.yml`](../docker/docker-compose-ci.yml) exactly match
those in [`.github/workflows/tests.yml`](../.github/workflows/tests.yml).

Then, run the container and work through the steps in the Github Actions file,
to see where things might have gone wrong.

```
# To start the container 
docker-compose -f docker/docker-compose-ci.yml run cli /bin/bash
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

This script will then set up the namespaces

```bash
sudo ./bin/netns.sh 20
```

## Cgroups

To use cgroup isolation, you'll need to run:

```bash
sudo ./bin/cgroup.sh
```

## Mounting your local build inside unmodified containers

If you need to debug issues with the standard containers, you can mount your 
local build of Faasm inside some otherwise unmodified containers as follows:

```
# --- Faasm CLI ---
# Build the code
inv dev.tools
```

Then, outside the container:

```
# Mount your local build inside the containers
export FAASM_BUILD_MOUNT=/build/faasm

# Start up the local cluster
cd faasm
docker-compose up -d
```

This will mount the built binaries from the Faasm CLI container into the other 
containers, thus allowing you to rebuild and restart everything with local 
changes. 

For example, if you have changed code and want to restart the worker container:

```
# Inside the CLI container, rebuild the pool runner (executed by the worker)
inv dev.cc pool_runner

# Outside the container, restart the worker
docker-compose restart worker

# Tail the logs
docker-compose logs -f
```

