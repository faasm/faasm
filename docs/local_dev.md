# Local Development

This guide is only relevant for those wanting to dig deeper or make changes to Faasm itself. 
If you'd just like to write and run functions, you can refer to the [set-up](setup.md) instructions. 

## Checking out the repo

For now, many locations rely on the code being located at `/usr/local/code/faasm`, with the latter 
two directories _owned_ by the current user. This is something that will be removed in future. 

You can either set this directory up directly, or just symlink it.

Assuming you've checked out this code somewhere, you'll need to make sure submodules are up to date:

```
git submodule init
git submodule update
```

## Basic local machine set-up

Most of the local set-up is scripted with Ansible, but you need to have Python 3 and 
[Ansible](https://www.ansible.com/) set up in advance.

The easiest way to do this is as follows:

```bash
# Python stuff
sudo apt install python3-dev python3-pip
sudo pip3 install -U pip

# Ansible
sudo pip install -U ansible

# Faasm python env 
source workon.sh
pip install -r faasmcli/requirements.txt

# Faasm playbook
cd ansible
ansible-playbook local_dev.yml --ask-become-pass
```

If you want to tweak things yourself, look inside the `local_dev.yml` playbook to see what's required.

### Protobuf

Faasm depends on protobuf which should be installed with the playbook described above.

If there are any issue you need to remove every trace of any existing protobuf installation on your system before 
reinstalling.

You can look in the following folders and remove any reference to `libprotobuf` or `protobuf`:

- `/usr/lib/x86_64-linux-gnu/`
- `/usr/lib/`
- `/usr/include/google`

Avoid trying to do this with `apt` as it can accidentally delete a whole load of other stuff.

## Toolchain and Runtime Root

The Faasm toolchain and runtime require some prebuilt files which can be downloaded with:

```bash
source workon.sh
inv toolchain.download-toolchain
inv toolchain.download-sysroot
```

If you want to build the toolchain from scratch, you'll need to look at the `toolchain.md` doc.

## Codegen and upload

To run the next parts you'll need to build the following targets with CMake:

- `codegen_func`
- `codgen_shared_obj`

Once finished, you need to add the resulting `bin` dir to your `$PATH`.

You can do this via your chosen IDE or with something like:

```bash
mkdir -p build/faasm && cd build/faasm
cmake -GNinja -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ../..
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

If you want to switch on network isolation locally, you need to set up network namespaces. To do this we need to
ensure consistent interface naming (`eth0` for main public interface). If your public interface is already called
`eth0` then you can skip this step.

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

# CLion/ JetBrains

## Python

The Python files for Faasm are all contained in the nested `faasmcli` project. To get JetBrains IDEs
to correctly resolve imports, you can do one of:

- Mark the top-level `faasmcli` directory as a "Project sources" or equivalent in your IDE
- [Add an interpreter path](https://www.jetbrains.com/help/pycharm/installing-uninstalling-and-reloading-interpreter-paths.html#add). 
- Open the IDE directly in the `faasmcli` directory

Note that you don't need to install it to use `inv` from the Faasm CLI, as this is set up to directly
import the files (provided you've used `workon.sh` as described in the [setup](setup.md)).

## Remote development using CLion

CLion supports [remote development](https://www.jetbrains.com/help/clion/remote-development.html) which 
can be useful for developing Faasm inside a VM (or some other remote location). CLion will upload the 
code to a temporary location in the remote environment (`/tmp/tmp.XXX`), which you must then symlink 
to `/usr/local/code/faasm` as described above. 

Ensure the toolchain within CLion is pointing at the correct remote version of Clang that is installed 
as part of the Faasm Ansible playbook (i.e. `/usr/bin/clang(++)`), and possibly recent CMake.

I do **not** recommend setting a WASM/FAASM CMake profile because it will confuse your IDE a lot. 

Assuming you've gone through the steps outlined above _within the VM_, you should be able to call the 
`inv` commands as normal (rather than needing to use the Faasm CLI container within the VM).
