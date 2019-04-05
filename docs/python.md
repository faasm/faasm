# Python 

## Python 3.7

We need a native version of exactly the right Python 3.7, _not_ necessarily the latest.
You need to install it with the `python3_7.yml` playbook. It will install a copy of python
at `/usr/local/faasm/python3.7`

```
cd ansible
ansible-playbook python3_7.yml
```

## Building CPython and packages

You must make sure that you've set up the latest emscripten toolchain locally, and built our own custom musl.

Once this is done, you can run the following:

```
cd pyodide/cpython
source bash_env.sh
make
cd ../packages
make
```

## Dynamic linking

Once this is all built we can generate the machine code up-front for all the relevant shared objects.

This is hacked together a bit at the moment with the following process:

```
# Generate the list of files (found in python/python_shared_obj.txt)
./bin/find_python_shared_obj.sh

# Run the code gen
inv python-codegen
```

## Setting up the runtime root

To run Python you need to set up the Faasm runtime root filesystem. To do this you can run:

```
./bin/set_up_python_root.sh
```

## Compiling Python functions


When building you **must** use the custom pyodide emscripten:

```
source /usr/local/code/faasm/pyodide/emsdk/emsdk/emsdk_env.sh
```

Once this is done, you can run the `upload` server and do the following:

```
inv funcs-python --func=<your func>
inv upload-func <your user> <your func> --python
```

# Modifications

## Tests

Can completely remove the `Lib/test` directory (and potentially anything else we don't need)
