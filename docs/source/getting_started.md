# Getting started

Note that if you want to develop Faasm and make changes, you need to run through
the setup in the [development docs](development.md).

## Containers

Faasm runs a few different containers as defined in
[docker-compose.yml](../../docker-compose.yml). The most important are:

- `cpp` - used to compile and upload C/C++ functions (see the [`cpp`
  repo](https://github.com/faasm/cpp)).
- `python` - used to compile the CPython runtime and libraries, and
  upload Python functions (see the [`python`
  repo](https://github.com/faasm/python)).
- `upload` - the server that receives all function, state and shared file
  uploads.
- `worker` - one or more instances of the Faasm runtime that execute functions.
- `minio` - [Minio](https://min.io/) instance that holds uploaded functions and
  shared files.
- `redis` - [Redis](https://redis.io/) instance that holds internal scheduler
  state, and state shared between functions.
- `nginx` - [NGINX](https://www.nginx.com/) instance that load-balances requests
  between multiple workers.

Faasm generates machine code for all WebAssembly it needs to run (e.g. C/C++
functions, CPython, Python C-extensions). This, along with other local
development artifacts, is stored in the `dev` dir at the root of your local
checkout.

## Python Virtual Environment

All Faasm functionality is accessible through Python tasks with [Invoke](
https://www.pyinvoke.org/). In order to install all the dependencies, make sure
you always have an active python virtual enviornment:

```bash
source ./bin/workon.sh
```

then, you can do the following:

```bash
# List all commands
inv -l

# List all commands in the k8s namespace
inv -l wast

# Show help for a given command
inv -h wast.decompile

# Chain commands
inv wast run
```

## Faasmctl

Faasm uses [`faasmctl`](https://github.com/faasm/faasmctl) to interact with a
running Faasm cluster.

`faasmctl` is installed automatically in the virtual environment.
In order to use it, you need to set an environment variable pointing to the
cluster's INI file:

```bash
export FAASM_INI_FILE=./faasm.ini
```

for more information on `faasmctl`, or details of its inner-workings, check
the [corresponding docs](https://github.com/faasm/faasmctl/tree/main/docs).

## Local cluster

To start a local deployment, you can run:

```
faasmctl deploy.compose
```

## Writing and deploying functions

See the language-specific docs:

- [C/C++](cpp.md)
- [Python](python.md),

### Flushing

Faasm does a lot of caching, so if you want to update a function, you must flush
the system before invoking it again. This is done using the `flush` command.
Each language-specific container has its own way of flushing (e.g. `inv
func.flush` in the `cpp` container), but you can also do it from outside all
containers with `faasmctl`:

```bash
faasmctl flush.all
```

The process for updating and invoking an updated function is:

- Modify and recompile the function
- Upload the function
- Flush
- Invoke function and see updated version
