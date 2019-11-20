# Local Development

Below are instructions for building, testing and developing.

## Tools/ system deps

Python, [Ansible](https://www.ansible.com/) and [Invoke](http://docs.pyinvoke.org/en/1.2/index.html) are required along with a couple of system dependencies.

The easiest way to install all of them is as follows:

```
sudo pip install -U ansible
cd ansible
ansible-playbook local.yml --ask-become-pass

sudo pip3 install invoke
```

## Submodules

Faasm relies on some submodules. You need to run the following:

```
git submodule init
git submodule update
```

## Libraries from source

Faasm has various library dependencies that need to be installed from source:

```
cd ansible
ansible-playbook --ask-become-pass \
   aws_lambda.yml \
   aws_sdk.yml \
   catch.yml \
   eigen.yml \
   local.yml \
   pistache.yml \
   rapidjson.yml \
   spdlog.yml
```

### Protobuf

Faasm depends on protobuf which can be a bit of a hassle to install. First of all you can try the task:

```
ansible-playbook protobuf.yml --ask-become-pass
```

If there are any issue you need to remove every trace of protobuf on your system before reinstalling.

You can look in the following folders and remove any reference to `libprotobuf` or `protobuf`:

- `/usr/lib/x86_64-linux-gnu/`
- `/usr/lib/`
- `/usr/include/google`

Avoid trying to do this with `apt` as it can accidentally delete a whole load of other stuff.

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

## Toolchain

The Faasm toolchain currently requires a custom build of LLVM. More info can be found in the `toolchain.md` file in this project. You need to run:

```
inv download-toolchain
```

## Codegen and Upload

To run the next parts you'll need to build the following targets:

- `codegen_func`
- `codgen_shared_obj`
- `upload` 

If doing an out of tree build with CMake, put it in a new `build` subdirectory so the python scripts can find the executables.

## Codegen for C++ functions

To run the codegen for all the C++ functions:

```
inv run-local-codegen
```

## Runtime files and Python

You can pull down the prepackaged python runtime and required runtime files with:

```
inv download-runtime-root
inv run-local-codegen
```

You can then put the Python functions in place with:

```
inv upload-all --py --local-copy
```

## Tensorflow data

To allow running the demo TensorFlow function, you need to run:

```
inv tf-upload-data --local-copy
```

## Tests

Once everything is set up you can generate the test files and run the tests via the `tests` target:

```
inv compile-libfake
```
