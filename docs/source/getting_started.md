# Getting started

Faasm can be accessed from the commandline via the containerised Faasm
environment:

```bash
./bin/cli.sh faasm
```

This will mount your local project checkout into the containerised development
setup (more info on developing Faasm can be found in the [dev
docs](development.md).

## Commandline interface

The Faasm CLI uses [Invoke](https://www.pyinvoke.org/), which means you can do
the following:

```bash
# List all commands
inv -l

# List all commands in the knative namespace
inv -l knative

# Show help for a given command
inv -h invoke

# Chain commands
inv flush invoke demo hello
```

## Local cluster

To start a local deployment, you can run:

```
docker compose up
```

which creates the containers defined in
[docker-compose.yml](../docker-compose.yml):

Faasm will generate machine code from all WebAssembly it encounters. This, along
with other local development artifacts are stored in the `dev` dir at the root
of your local checkout.

## Writing and deploying functions

See the language-specific docs for [C/C++](cpp.md) and [Python](python.md),

### Flushing

Faasm does a lot of caching, so if you want to update a function, you must flush
the system before invoking it again. This is done using the `flush` command.
Each language-specific container has its own way of flushing (e.g. `inv
func.flush` in the `cpp` container), but you can also do it from the CLI.

```bash
inv flush
```

The process for updating and invoking an updated function is:

- Modify and recompile the function
- Upload the function
- Flush
- Invoke function and see updated version
