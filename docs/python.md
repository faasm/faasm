# Python 

## Python 3.7

We need a native version of exactly the right Python 3.7, _not_ necessarily the latest. You need to install it with the `python3_7.yml` playbook,

```
cd ansible
ansible-playbook python3_7.yml --ask-become-pass
```

## Building CPython

We build Python using [pyodide](https://github.com/iodide-project/pyodide) which is included as a submodule.

The first pyodide build takes a while. To set things up you can do the following:

```
git submodule update
export EMCC_WASM_BACKEND=1
cd pyodide/emsdk
make
cd ../cpython
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
