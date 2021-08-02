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

## Enabling Python support

Full Python support is **not enabled by default**. To enable the Python runtime
you must set up the relevant environment variables:

```bash
# On the "upload" container/ endpoint (see docker-compose.yml locally)
PYTHON_CODEGEN=on
```

The first time the system runs it will generate machine code for python and all
the Python C-extensions. This can take up to a couple of minutes depending on
your machine, but is a one-off job.

## Running a Python function

An example Python function is found at `func/python/hello.py`. This can be
uploaded and invoked from the Python CLI with:

```bash
# Run the Python CLI
./bin/cli.sh python

# Build and upload the Python runtime
inv func
inv func.upload

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
