# Faasm Development

This guide is only relevant for those wanting to dig deeper or make changes to
Faasm itself. If you'd just like to write and run functions, you can refer to
the [set-up instructions](setup.md). 

## Recommended Set-up - Docker CLI

Most development can be done using the containerised CLI. To get everything set
up, you need to run:

```bash
# Clone this repo
git clone https://github.com/faasm/faasm
cd faasm

# Start the CLI
./bin/cli.sh
```

The service orchestration is handled through `docker-compose`. If you want to
stop all `faasm`-related development services run:

```bash
docker-compose down
```

To build and run the tests, you can then run the following inside the container:

```bash
# Build the development tools
inv dev.tools

# Upload the Python functions
inv upload.user python --local-copy --py

# Run codegen (this may take a while the first time it's run)
inv codegen.local
inv python.codegen

# Set up cgroup
./bin/cgroup.sh

# Run the tests
tests
```

### Tooling - editors, IDEs etc.

Several directories (including the project root) are mounted as volumes in the
CLI container (see [`docker-compose.yml`](../docker-compose.yml)). This means
you can edit the files directly on your host and compile them inside the
container.

You can use remote development tools to develop and debug inside the CLI
container (e.g.  [CLion remote
development](https://www.jetbrains.com/help/clion/remote-development.html) or
[`gdbserver`](https://sourceware.org/gdb/onlinedocs/gdb/Server.html)).

You can also create a new container that inherits from the CLI if you want to
set up your own. To do this:

```bash
./bin/cli.sh <your image>
```

Or set the environment variable `CLI_IMAGE`  before you run the `cli.sh` script.

## Testing

The tests use [Catch2](https://github.com/catchorg/Catch2) and your life will be
much easier if you're familiar with their [command line
docs](https://github.com/catchorg/Catch2/blob/v2.x/docs/command-line.md).  This
means you can do things like:

```
# Run all the MPI tests
tests "[mpi]"

# Run a specific test
tests "Test some feature"
```

### Troubleshooting CI

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

## Building outside of the container

If you want to build the project outside of the recommended container, or just
run some of the CLI tasks, you can take a look at the [CLI
Dockerfile](../docker/cli.dockerfile) to see what's required:

To run the CLI, you should just need to do:

```bash
# Set up some shortcuts and aliases
export FAASM_BUILD_DIR=$(pwd)/build
source bin/workon.sh

# Activate the virtualenv
source venv/bin/activate

# Install toolchain Python module
cd <wherever you keep your code>
git clone https://github.com/faasm/faasm-toolchain/
cd faasm-toolchain
pip3 install -e .
cd -

# Install Faasm CLI module
cd faasmcli
pip install -e .
cd ..

# Check things work
inv -l
```

From then on, you should just be able to run the following (i.e. not reinstall
the python modules):

```
source bin/workon.sh

inv -l
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

We also use [`clang-tidy`](https://clang.llvm.org/extra/clang-tidy/) as a
linter and static analyzer. To set it up for the project, you need to
generate a compilation database. This can be done as follows:
```bash
cd <FAASM_REPO>
mkdir -p build; cd build
cmake --DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

## Detailed notes

### Networking

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

### Cgroups

To use cgroup isolation, you'll need to run:

```bash
sudo ./bin/cgroup.sh
```

# Out-of-tree build example

This is a simple example of running an out-of-tree build to execute a 
function:

```bash
# Set up the CLI
export FAASM_BUILD_DIR=<build dir>
source bin/workon.sh

# Build the project
inv dev.tools

# Compile, codegen and run the code
inv compile demo hello
inv codegen demo hello
inv run demo hello
```

