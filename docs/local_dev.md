# Local Development

This guide is only relevant for those wanting to dig deeper or make changes to Faasm itself. If you'd just like to write 
and run functions, then the Docker set-up described in the README should be sufficient. This includes deploying to 
Knative or other cloud providers. 

## Submodules

Assuming you've checked out this code somewhere, you'll need to make sure submodules are up to date:

```
git submodule init
git submodule update
```

## Basic local machine set-up

Most of the local set-up is scripted with Ansible, but you need to have Python 3, [Ansible](https://www.ansible.com/) 
and [Invoke](http://docs.pyinvoke.org/en/1.2/index.html) set up in advance.

The easiest way to do this and run the local dev set-up is as follows:

```
sudo pip install -U ansible
cd ansible
ansible-playbook local_dev.yml --ask-become-pass

sudo pip3 install invoke
```

### Protobuf

Faasm depends on protobuf which should be installed with the playbook described above.

If there are any issue you need to remove every trace of any existing protobuf installation on your system before 
reinstalling.

You can look in the following folders and remove any reference to `libprotobuf` or `protobuf`:

- `/usr/lib/x86_64-linux-gnu/`
- `/usr/lib/`
- `/usr/include/google`

Avoid trying to do this with `apt` as it can accidentally delete a whole load of other stuff.

## Toolchain, codegen etc.

The Faasm toolchain currently requires a custom build of LLVM. You can run the following to download the prebuilt 
version:

```
inv download-toolchain
inv download-sysroot
```

If you want to build the toolchain from scratch, you'll need to look at the `toolchain.md` doc.

## Codegen and upload

To run the next parts you'll need to build the following targets:

- `codegen_func`
- `codgen_shared_obj`
- `upload` 

If doing an out of tree build with CMake, put it in a new `build` subdirectory so the python scripts can find the 
executables.

Run the `upload` target to start an upload server.

### Codegen for C++ functions

To run codegen for all the C++ functions:

```
inv run-local-codegen
```

### Python functions

You can pull down the prepackaged python runtime and required runtime files with:

```
inv download-runtime-root
inv run-local-codegen
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
