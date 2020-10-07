# Faasm Development

This guide is only relevant for those wanting to dig deeper or make changes to
Faasm itself. If you'd just like to write and run functions, you can refer to
the [set-up](setup.md) instructions. 

## Recommended Set-up

The Faasm codebase is all standard C/C++ and Python, so should work on a range 
of set-ups, however, the recommended configuration is:

- Ubuntu 20.04
- Python 3.8
- Clang 10
- CMake 3.13+
- Ninja (rather than Make)

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

## Checking out the repo

For now, many locations rely on the code being located at
`/usr/local/code/faasm` (note the lowercase "f"), with the latter two 
directories _owned_ by the current user. This is annoying and something we will 
fix in future. 

You can either set this directory up directly, or just symlink it.

Assuming you've checked out this code somewhere, you'll need to make sure
submodules are up to date:

```bash
git submodule update --init
```

## Basic local machine set-up

Most of the local set-up is scripted with Ansible, but you need to have Python 3
and [Ansible](https://www.ansible.com/) set up in advance.

The easiest way to do this is as follows:

```bash
# Python stuff and other dependencies
sudo apt install python3-dev python3-pip python3-venv libcairo2-dev libtinfo5
sudo pip3 install -U pip

# Ansible
sudo pip install -U ansible

# Faasm python env 
source workon.sh
pip install -r faasmcli/requirements.txt

# Faasm CLI
pip install -e faasmcli/

# Faasm playbook
cd ansible
ansible-playbook local_dev.yml --ask-become-pass
```

If you want to tweak things yourself, look inside the `local_dev.yml` playbook
to see what's required.

### Protobuf and gRPC

Faasm depends on Protobuf and gRPC which are set up with the Ansible `grpc` role
found [here](ansible/roles/grpc/tasks/main.yml). However, if you want to intsall
these yourself or use an existing version, make sure you use the full 
[gRPC C++ build](https://grpc.io/docs/languages/cpp/quickstart/).

## Toolchain and Runtime Root

The Faasm toolchain and runtime require some prebuilt files which can be
downloaded with:

```bash
source workon.sh
inv toolchain.download-toolchain
inv toolchain.download-sysroot
```

If you want to build the toolchain from scratch, you'll need to look at the
[toolchain README](https://github.com/faasm/faasm-toolchain/README.md) doc.

## Codegen and upload

To generate machine code for your system you'll need to build a couple of Faasm
targets with:

```bash
inv dev.cmake
inv dev.cc codegen_func
inv dev.cc codegen_shared_obj
```

The Python source for these commands can be found in the [CLI
source](faasmcli/faasmcli/tasks/dev.py).

### Python functions

You can pull down the prepackaged python runtime and required runtime files
with:

```bash
inv toolchain.download-runtime
```

You can then put the Python functions in place with:

```bash
inv upload.user python --py --local-copy
```

## Networking

If you want to switch on network isolation you need to set up network
namespaces. To do this we need to ensure consistent interface naming (`eth0` for
main public interface). If your public interface is already called `eth0` then
you can skip this step.

- Edit `/etc/default/grub` and add `net.ifnames=0 biosdevname=0` to `GRUB_CMDLINE_LINUX_DEFAULT`
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

# Troubleshooting the local dev set-up

This section consists of issues that may occure during installation and possible
solutions.

## Error 'bdist_wheel' can't be found after invoking pip install -r faasmcli/requirements.txt

```bash
# Remove installed requirements
pip uninstall -r faasmcli/requirements.txt

# Install setuptools and wheel 
pip install setuptools wheel

#Install faasmcli requirements
pip install -r faasmcli/requirements.txt
```

