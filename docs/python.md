# Python 

## Python 3.7

If you don't already have python3.7 available, you need to install it with the `python3_7.yml` playbook,

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

## Modified functions

The following functions are declared with a single object pointer, but need varargs added:

```
// From this
static PyObject *
func_get_name(PyFunctionObject *op)

// To this
static PyObject *
func_get_name(PyFunctionObject *op, ...)
```

- func_get_name (funcobject.c)
- func_set_name (funcobject.c)
- func_get_qualname (funcobject.c)
- func_set_qualname (funcobject.c)
- dictitems_new (dictobject.c x2)
- thread_PyThread_allocate_lock (_threadmodule.c)
- thread_get_ident (_threadmodule.c)
- stdprinter_noop (fileobject.c)