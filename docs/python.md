# Python 

## Python 3.7

If you don't already have python3.7 available, you need to install it with the `python3_7.yml` playbook,

```
cd ansible
ansible-playbook python3_7.yml --ask-become-pass
```

## Building CPython

We build Python using [pyodide](https://github.com/iodide-project/pyodide).

Unfortunately pyodide bundles its own `emsdk` which we must build with:

```
inv build-pyodide-emscripten
```

This will take ages but is a one-off. Once done, we can build cpython with:

```
source ~/faasm/pyodide/emsdk/emsdk/emsdk_env.sh
inv build-pyodide-cpython
```

## Setting up the runtime root

To run Python you need to set up the Faasm runtime root filesystem. To do this you can run:

```
sudo mkdir /usr/local/faasm/runtime_root
sudo chown -R <user>:<user> !$
cd /usr/local/code/faasm
./bin/set_up_python_root.sh
```

## Compiling Python functions

To compile Python functions we need to use the pyodide version of Emscripten. To do this, you
**must** run the following in your shell:

```
source ~/faasm/pyodide/emsdk/emsdk/emsdk_env.sh
```

Once this is done, you can run:

```
inv funcs-python --func=<your func>
inv upload-func <your user> <your func> --python
```
