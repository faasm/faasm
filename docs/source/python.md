# Python

Faasm executes functions compiled to WebAssembly, which rules out _directly_
executing languages which can't be compiled to WebAssembly. However, we can
compile certain language runtimes to WebAssembly, then use these to execute
non-WebAssembly code.

This is true of python, which we can compile to WebAssembly (along with Python
C-extensions). This is covered in the [Faasm python
repo](https://github.com/faasm/python).

A separate Python library, [Pyfaasm](https://github.com/faasm/pyfaasm),
contains the custom C-extensions and decorators to support the [Faasm host
interface](host_interface.md).

## Quick start

Start a cluster with:

```bash
export PYTHON_CODEGEN=on

faasmctl deploy.compose
```

Upload and invoke a Python function with:

```
# Run the Python CLI
faasmctl cli.python

# Build and upload the Python runtime (CPython cross-compiled to WebAssembly)
inv cpython.func
inv cpython.upload

# Upload the hello function
inv func.uploadpy hello

# Invoke the hello function
inv func.invoke python hello
```

This should give a message and the version of Python being run.

## Python API

A simple example of chaining Python functions in Faasm looks like:

```python
from pyfaasm.core import await_call, chain_this_with_input

def func_one(data):
    pass

def func_two(data):
    pass

def faasm_main():
    call_one = chain_this_with_input(func_one, b'')
    call_two = chain_this_with_input(func_two, b'')

    await_call(call_one)
    await_call(call_two)
```

## Updating the Python runtime

If you are updating the Python runtime (i.e. CPython) and you want to test the
changes you will need to re-generate machine code for both the runtime and the
shared libraries:

```bash
inv codegen python py_func [--clean]
inv python.codegen [--clean]
inv python.clear-runtime-pyc
```
