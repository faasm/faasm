# Local Development

This guide is only relevant for those wanting to dig deeper or make changes to
Faasm itself.  If you'd just like to write and run functions, you can refer to
the [set-up](setup.md) instructions. 

## Recommended Set-up

The Faasm codebase is all standard C/C++ and Python, so should work on a range 
of set-ups, however, the only one that is recommended and well tested is:

- Ubuntu 18.04
- Python 3.7
- Clang, not GCC

All scripted CMake builds also use Ninja, so it's safest to use that too.

This means your CMake commands will look something like:

```
cmake \
    -GNinja \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
    -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
    ...
```

## Checking out the repo

For now, many locations rely on the code being located at
`/usr/local/code/faasm` (note the lowercase "f"), with the latter two 
directories _owned_ by the current user. This is annoying and something we will 
fix in future. 

You can either set this directory up directly, or just symlink it.

Assuming you've checked out this code somewhere, you'll need to make sure
submodules are up to date:

```
git submodule update --init
```

## Basic local machine set-up

Most of the local set-up is scripted with Ansible, but you need to have Python 3 and 
[Ansible](https://www.ansible.com/) set up in advance.

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

### Protobuf

Faasm depends on protobuf which should be installed with the playbook described
above.

You can probably get away with using whatever protobuf you already have
installed too.

If there are any issue you need to remove every trace of any existing protobuf
installation on your system before reinstalling.

You can look in the following folders and remove any reference to `libprotobuf`
or `protobuf`:

- `/usr/lib/x86_64-linux-gnu/`
- `/usr/lib/`
- `/usr/include/google`

Avoid trying to do this with `apt` as it can accidentally delete a whole load of
other stuff.

To rerun just the protobuf part of the install:

```
cd ansible
ansible-playbook protobuf.yml --ask-become-pass
```

## Toolchain and Runtime Root

The Faasm toolchain and runtime require some prebuilt files which can be
downloaded with:

```bash
source workon.sh
inv toolchain.download-toolchain
inv toolchain.download-sysroot
```

If you want to build the toolchain from scratch, you'll need to look at the
`toolchain.md` doc.

## Codegen and upload

To run the next parts you'll need to build the following targets with CMake:

- `codegen_func`
- `codegen_shared_obj`

Once finished, you need to add the resulting `bin` dir to your `$PATH`.

You can do this via your chosen IDE or with something like:

```bash
mkdir -p build/faasm && cd build/faasm
cmake -GNinja -DCMAKE_C_COMPILER=/usr/bin/clang-10 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 ../..
cmake --build . --target codegen_func
cmake --build . --target codegen_shared_obj

export PATH=$(pwd)/bin:$PATH
```

### Codegen for C++ functions

To run codegen for all the C++ functions:

```
inv codegen.local
```

### Python functions

You can pull down the prepackaged python runtime and required runtime files with:

```
inv toolchain.download-runtime
```

You can then put the Python functions in place with:

```
inv upload.user python --py --local-copy
```

## Networking

If you want to switch on network isolation locally, you need to set up network
namespaces. To do this we need to ensure consistent interface naming (`eth0` for
main public interface). If your public interface is already called `eth0` then
you can skip this step.

- Edit `/etc/default/grub` and add `net.ifnames=0 biosdevname=0` to `GRUB_CMDLINE_LINUX_DEFAULT`
- Run `sudo update-grub`
- Restart the machine

This script will then set up the namespaces

```
sudo ./bin/netns.sh 20
```

## Cgroups

To use cgroup isolation, you'll need to run:

```
sudo ./bin/cgroup.sh
```

# Out-of-tree build example

This is a simple example of running an out-of-tree build to execute a 
function:

```bash
# Normal CMake set-up
mkdir build 
cd build
cmake -GNinja \
  -DCMAKE_C_COMPILER=/usr/bin/clang-10 \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 \
  .. 

# Build upload and simple_runner and codegen function
cmake --build . --target simple_runner
cmake --build . --target codegen_func

# Set up the shell environment in the top-level directory
cd ..
source workon.sh

# Compile, codegen and run the code
inv compile demo hello
inv codegen demo hello
inv run demo hello
```

# CLion/ JetBrains

## Python

The Python files for Faasm are all contained in the nested `faasmcli` project.
To get JetBrains IDEs to correctly resolve imports, you can do one of:

- Mark the top-level `faasmcli` directory as a "Project sources" or equivalent in your IDE
- [Add an interpreter path](https://www.jetbrains.com/help/pycharm/installing-uninstalling-and-reloading-interpreter-paths.html#add). 
- Open the IDE directly in the `faasmcli` directory

# Troubleshooting the local dev set-up

This section consists of issues that may occure during installation and possible solutions.

## Error 'bdist_wheel' can't be found after invoking pip install -r faasmcli/requirements.txt

```bash
# Remove installed requirements
pip uninstall -r faasmcli/requirements.txt

# Install setuptools and wheel 
pip install setuptools wheel

#Install faasmcli requirements
pip install -r faasmcli/requirements.txt
```

