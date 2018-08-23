# Usage

## Invoking Functions

Each function is associated with a user and has a function name. It will have two URLs:

- Synchronous - `<faasm_host>/f/<user>/<function>/`
- Asynchronous - `<faasm_host>/fa/<user>/<function>/`

By `POST`ing to these URLs we can invoke the function.

For example with the faasm endpoint at `localhost:8080`, to test the `echo` function owned by user `simon` we can run:

```
curl -X POST http://localhost:8080/f/simon/echo -d "hello faasm"
```

We should then see a response containing our input data (i.e. `hello faasm`).

## Adding Functions

To upload a function you can use `curl` to send a PUT request to the synchronous URL for the given function.
For example, if I:

- Have a faasm endpoint running at `localhost:8080`
- Have a compiled function file at `/tmp/do_something.wasm`
- Want to upload this function to user `simon` and function name `cool_func`

I would execute:

```
curl http://myhost:8080/f/simon/cool_func/ -X PUT -T /tmp/do_something.wasm
```

# Kubernetes

Faasm can be deployed to Kubernetes to create a distributed set-up. The configuration files are found in the `k8s`
directory. 

## Deploying to Kubernetes

Deployment to Kubernetes is handled via the make target:

```
make setup-k8
```

# Installation, Configuration and Development

## Local Development

### Submodules

Faasm relies on WAVM which needs to be updated via a Git submodule

```
# Update submodules
git submodule update --remote --init
```

### Libraries

Faasm has various library dependencies that can be installed with Ansible using the relevant `make` target:

```
make setup-libs
```

### Protobuf

Faasm depends on protobuf which can be a bit of a hassle to install. First of all you can try the make target:

```
make setup-protobuf
```

If there are any issue you need to remove every trace of protobuf on your system before reinstalling.

You can look in the following folders and remove any reference to `libprotobuf` or `protobuf`:

- `/usr/lib/x86_64-linux-gnu/`
- `/usr/lib/`
- `/usr/include/google`

Avoid trying to do this with `apt` as it can accidentally delete a whole load of other stuff.

### Clang

I've only used clang to build faasm so far. There's a make target for it:

```
make setup-clang
```

### Building with CMake

Faasm also requires an up-to-date version of CMake. Once you have this you can run the following from the root of
your project:

```
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target all
```

I usually develop through CLion which does a lot of this stuff for you.
