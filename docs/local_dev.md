# Local Development

Below are instructions for building, testing and developing.

## Tools/ system deps

Python, [Ansible](https://www.ansible.com/) and [Invoke](http://docs.pyinvoke.org/en/1.2/index.html) are required
along with a couple of system dependencies.

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
sudo ./bin/netns.sh
```

## Cgroups

To use cgroup isolation, you'll need to run:

```
sudo ./bin/cgroup.sh
```

## Emscripten

We're currently pinned to a specific version of the `emsdk` build of the
wasm toolchain. This is stored in S3. To pull it you can run:

```
inv restore-emsdk
```

It then lives at `/usr/local/faasm/emsdk`. To update it to the latest version
you can run:

```
cd pyodide/emsdk

# Update the version in the Makefile to the latest, then...
make
```

You can then copy this to `/usr/local/faasm/emsdk` and run:

```
inv backup-emsdk
```

## Libc

We use our own custom musl port with a custom malloc, which can be built and installed by running:

```
./bin/build_musl.sh
inv compile-malloc
```

## Libfaasm

To set up our library utilities, run:

```
inv compile-libfaasm
```

## Python runtime

You an pull down the prepackaged python runtime using:

```
inv download-python-runtime
```

## Docker images

The Docker images are structured to minimise rebuilding time. They are as follows:

- `faasm/cpp-base` - minimal C++ tooling and protobuf
- `faasm/base` - includes all dependencies and WAVM
- `faasm/worker`, `faasm/upload` and `faasm/edge` - hold the different application components
- `faasm/toolchain` - stand-alone container holding the toolchain

These can be rebuilt as follows:

```
# Build base container if any changes to WAVM or library dependencies
inv build-base

# Build the containers themselves
inv build-worker build-edge build-upload
```

## Tests

Tests can be run with the `tests` target of this project.

If things are failing you can run `bin/set_up_local.sh` to rerun a couple of tidy-ups.
