# Python 

## Python 3.7

If you don't already have python3.7 available, you need to install it with the `python3_7.yml` playbook,

```
cd ansible
ansible-playbook python3_7.yml --ask-become-pass
```

## Building CPython

We build Python using [pyodide](https://github.com/iodide-project/pyodide).

The first pyodide build takes a very long time and can be run with their Docker set-up as described in the README.

To run locally you need to do the following 

```
cd /usr/local/code
git clone git@github.com:iodide-project/pyodide.git
cd pyodide

# Check out a stable release
git checkout 0.9.0

./run_docker
make
```

This will perform the build in place so you can access it from your local filesystem.

Once it's finished, it might be useful to change the whole thing to be owned by you:

```
sudo chown -R $USER:$USER /usr/local/code/pyodide
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
source /usr/local/code/pyodide/emsdk/emsdk/emsdk_env.sh
```

Once this is done, you can run the `upload` server and do the following:

```
inv funcs-python --func=<your func>
inv upload-func <your user> <your func> --python
```
