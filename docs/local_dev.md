# Local Development

This guide is only relevant for those wanting to dig deeper or make changes to Faasm itself. If you'd just like to write 
and run functions, then the Docker set-up described in the README should be sufficient. This includes deploying to 
Knative or other cloud providers. 

## Checking out the repo

Whether you are working remotely (see below) or locally, a lot of **default parameters** assume that the repository is
located in `/usr/local/code/faasm`, the latter two directories _owned_ by the current user. To avoid having to set up
environment variables in the many scripts you will use to get started, we recommend to **symlink** where you or your IDE
(if using remote development) checked out the code to this location.

Assuming you've checked out this code somewhere, you'll need to make sure submodules are up to date:

```
git submodule init
git submodule update
```

## Basic local machine set-up

Most of the local set-up is scripted with Ansible, but you need to have Python 3 and [Ansible](https://www.ansible.com/) set up in advance.

The easiest way to do this is as follows:

```bash
# Python stuff
sudo pip install python3-dev python3-pip
sudo pip3 install -U pip

# Ansible
sudo pip install -U ansible

# Faasm python env 
source workon.sh
pip install -r requirements.txt

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
inv download-toolchain
inv download-sysroot
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
inv run-local-codegen
```

### Python functions

You can pull down the prepackaged python runtime and required runtime files with:

```
inv download-runtime-root
```

You can then put the Python functions in place with:

```
inv upload-all --py --local-copy
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

## Remote development using CLion

The first step you should be doing when CLion uploads the code to a temporary location (e.g. `/tmp/tmp.XXX`) is to
symlink this location to `/usr/local/code/faasm` as explained above for consistency (e.g. VM crash).

This can be a two step process where you first set-up the dependencies to locally on the VM then decide to edit the code
remotely using CLion by letting the IDE re-upload the code. This works quite well as CLion will not get confused with
bad local toolchains and should build the index pretty much out of the box if you have set up a remote toolchain to
your VM and set it to be the default one - you might want to tweak `update-alternatives` for `cc` and `c++` to use
`clang-9` however, or set the `C/CXX` cmake compiler flags manually to use `/usr/bib/clang(++)` which as defined by the
playbook should point to the version of `clang` used by Faasm).

I do **not** recommend setting a WASM/FAASM CMake profile because it will confuse your IDE a lot. To build the WASM
functions, simply ssh in to your VM at `/usr/local/faasm/code` and run the related `inv` commands (assuming you've
downloaded the toolchain and sysroot as described above).
