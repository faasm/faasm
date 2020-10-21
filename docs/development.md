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

# Update submodules
git submodule update --init

# Start the CLI
./bin/cli.sh
```

To build and run the tests, you can then run the following inside the container:

```bash
# Build the tests and the codegen targets
inv dev.cmake
inv dev.cc tests
inv dev.cc codegen_func
inv dev.cc codegen_shared_obj

# Create a cgroup
./bin/cgroup.sh

# Upload the Python functions (one-off)
inv upload.user python --local-copy --py

# Run codegen (this may take a while, one-off)
inv codegen.local

# Run the tests
tests
```

### Tooling - editors, IDEs etc.

Your checkout will be mounted as a volume in the container, so you can edit the
files locally and recompile them within the container.

You can use remote development tools against the running container (e.g. 
[CLion remote development](https://www.jetbrains.com/help/clion/remote-development.html) or 
[`gdbserver`](https://sourceware.org/gdb/onlinedocs/gdb/Server.html)).

You can also create a new container that inherits from the CLI if you want to
set up your customisations. To do this:

```bash
./bin/cli.sh <your image>
```

## Building outside of the container

If you want to build the project outside of the recommended container, or just
run some of the CLI tasks, you can take a look at the [CLI
Dockerfile](../docker/cli.dockerfile) to see what's required.

You will most likely need to set up the CLI using:

```bash
export FAASM_BUILD_DIR=$(pwd)/build
source bin/workon.sh
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
source workon.sh

# Build the project
inv dev.cmake --clean
inv dev.cc simple_runner
inv dev.cc codgen_func

# Compile, codegen and run the code
inv compile demo hello
inv codegen demo hello
inv run demo hello
```

