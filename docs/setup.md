# Usage and Set-Up

## CLI

All Faasm instances are accessible through the containerised CLI, which 
can be started with [cli.sh](../bin/cli.sh), i.e.

```bash
./bin/cli.sh
```

This will mount your current project root inside the container and set up the 
Faasm CLI environment. From inside this container you can perform all
interactions with a Faasm deployment. 

The Faasm CLI uses [Invoke](https://www.pyinvoke.org/), and a list of the 
available commands can be shown with:

```bash
inv -l
```

They are loosely collected into namespaces (of the format `xxx.yyy`) and 
there is tab completion (but it's a bit slow).

## Local cluster

To start a local deployment, you can run:

```
docker-compose up
```

which creates the containers defined in [docker-compose.yml](../docker-compose.yml):

Faasm will generate machine code from all WebAssembly it encounters. This is
stored in the `container/machine-code` directory at the root of this project.

## Writing and deploying functions

See the language-specific docs for [C/C++](cpp.md) and [Python](python.md).
