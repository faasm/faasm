# Python 

## Python 3.7

If you don't already have python3.7 available, you need to install it with the `python3_7.yml` playbook,

```
cd ansible
ansible-playbook python3_7.yml --ask-become-pass
```

## Building CPython

We build Python using [pyodide](https://github.com/iodide-project/pyodide).

The first pyodide build takes multiple hours and can be run with their Docker set-up.

To do this, you need to run the following. **NOTE we must check out a stable release**

```
cd /usr/local/code
git clone git@github.com:iodide-project/pyodide.git
cd pyodide
git checkout 0.9.0
./run_docker
make
```

This will perform the build in place so you can access it from your local filesystem.

## Setting up the runtime root

To run Python you need to set up the Faasm runtime root filesystem. To do this you can run:

```
./bin/set_up_python_root.sh
```

## Compiling Python functions


When building you **must** use their custom emscripten:

```
source /usr/local/code/pyodide/emsdk/emsdk/emsdk_env.sh
```

Once this is done, you can run:

```
inv funcs-python --func=<your func>
inv upload-func <your user> <your func> --python
```
