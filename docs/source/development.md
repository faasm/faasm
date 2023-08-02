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
need to update the project submodules (may take a while):

```bash
git submodule update --init --recursive
```

Then, you may start a development cluster mounting the checked-out code into
each service container, and sharing a system and runtime root filesystem.

```bash
faasmctl deploy.compose --mount-source . [--workers=0]

# Exporrt the path to the INI file, to avoid `faasmctl` asking for it again
export FAASM_INI_FILE=./faasm.ini
```

pass the `--workers=0` flag to avoid starting any workers (unnecessary for
day-to-day develeopment).

## Use

Once you've set up the repo, you should first run the Faasm CLI to make sure
things work:

```bash
faasmctl cli.faasm

# Check the python environment is up (should list commands)
inv -l

# Run the CMake build
inv dev.cmake
inv dev.tools
```

From there you can compile and run functions using the language-specific
containers (running the following from the local machine, not inside the Faasm
CLI container):

```bash
# C++ functions
faasmctl cli.cpp

# Python functions
faasmctl cli.python
```

## Tests

To run the tests, you can execute the following (note that you have to switch
between containers for the first few steps):

```bash
# --- CPP CLI ---
# Build CPP functions and lib required for the tests
inv func.local libfake

# --- Python CLI ---
# Build Python wrapper function and upload Python code
inv cpython.func func.upload-all --local

# --- Faasm CLI ---
# Build the development tools
inv dev.tools

# Run codegen (this may take a while the first time it's run)
inv codegen.tests

# Run the tests
inv tests

# You may also want to run groups of tests by name, by file, or by directory
inv tests [--test-case <test_name>]
inv tests [--test-file <file_name_no_extension>] # e.g. test_dynamic_linking
inv tests [--test-dir <directory_name>] # e.g. faaslet
```

## Tooling - editors, IDEs etc.

You can use custom containers that inherit from the existing CLI images if you
want to add text editors etc.

Before running the `faasmctl` script, you need to set one or more of the
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

> We add a wrapper script around the base `tests` command (see `inv tests`) to
> make sure all the correct env. variables for the tests are set. Using the
> underlying `tests` command should only be necessary in special cases (e.g.
> starting GDB).

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

To format code locally, or to check for formatting errors, you can run:

```bash
inv format-code [--check]
```

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
faasmctl deploy.compose [--num-workers=<num_workers>]
```

for more details on the `deploy.compose` command, check [faasmctl's docs](
https://github.com/faasm/faasmctl/tree/main/docs/deploy.md#docker-compose).

### Making changes in your local cluster

Assuming you've changed something related to the `pool_runner` target (which is
executed by the `worker` container), you can pick up the changes with:

```bash
faasmctl cli.faasm

# Rebuild the pool_runner target
inv dev.cc pool_runner
```

From a different terminal, restart the worker and check the logs:

```bash
faasmctl restart -s worker

faasmctl logs -s worker -f
```

### Running distributed tests locally

The distributed tests check the interactions of the Faasm workers, file storage,
redis and upload containers.

First of all, you need to make sure everything else is shut down to avoid
interfering with the tests:

```bash
faasmctl delete
faasmctl deploy.dist-tests --mount-source .
```

Make sure your local setup is built, along with the distributed tests:

```bash
faasmctl cli.faasm --cmd "./deploy/dist-test/build_internal.sh

faasmctl restart -s upload -s dist-test-server
```

Outside the container upload all the functions and run the tests:

```bash
# Upload everything
./deploy/dist-test/upload.sh

# Run the tests
./deploy/dist-test/run.sh
```

You can then rebuild and rerun from inside the container:

```bash
# Build tests and the server
inv dev.cc dist_tests dev.cc dist_test_server

# Run
dist_tests
```

If changing the server, you need to restart from outside the container:

```bash
faasmctl restart -s dist-test-server
```
