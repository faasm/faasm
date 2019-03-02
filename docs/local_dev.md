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

Faasm relies on WAVM as a git submodule, which will need to be updated when you first clone this project:

```
git submodule init
git submodule update
```

## Clang

You need to install clang using:

```
ansible-playbook --ask-become-pass clang.yml
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

### Emscripten

If you want to compile things with Emscripten, you can install it using:

```
ansible-playbook --ask-become-pass emscripten.yml
```

### wasm Toolchain

A Dockerised wasm toolchain is already configured. To enter a container with these tools you can run the following:

```
# Start the container
inv tools

# Check clang works
/toolchain/bin/clang -v
```

You can run the normal `inv` tasks from inside this container, e.g. to build libfaasm you can run

```
# From inside the tools
inv compile-libfaasm
```

## Building with CMake

You can run the following from the root of the project:

```
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target all
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
sudo ./bin/netns.sh
```

## Cgroups

To use cgroup isolation, you'll need to run:

```
sudo ./bin/cgroup.sh
```

## Running Locally

Once things are built, you can run a simple local set-up with:

```
# Set up function storage path (see below)
export FUNC_ROOT=<path to the root of this project>

# Edge node listening on localhost:8080
./build/bin/edge

# Worker
./build/bin/worker
```

The `FUNC_ROOT` is where Faasm will look for function files. For a function called `dummy` owned by user
`simon` the `.wasm` file should be found at `$FUNC_ROOT/wasm/simon/dummy/function.wasm`. When you upload a function
this is where it will get placed too.

## Docker images

There are a few Docker images used to make build times quicker:

- `shillaker/cpp-base` - from [this repo](https://github.com/Shillaker/cpp-base), just a base image that includes clang and protobuf (as they're a pain to install).
- `faasm/worker`, `faasm/upload` and `faasm/edge` - images used to actually run the application.

These can be rebuilt as follows:

```
# Build base container if any changes to WAVM or library dependencies
inv build-base

# Build the containers themselves
inv build-worker build-edge build-upload
```

## Tests

The tests can be found in the `test` directory and executed by running:

```
./build/bin/tests
```
