# Python 

Faasm executes functions compiled to WebAssembly, which therefore rules out 
languages that cannot be compiled to WebAssembly. However, we can support 
dynamic languages like Python by compiling _the language runtime_ to WebAssembly. 

In Faasm we do this with Python via a set of custom C-extensions and decorators to support the 
[Faasm host interface](host_interface.md) provided in [Pyfaasm](https://github.com/Shillaker/pyfaasm),
and use CPython compiled to WebAssembly with [Pyodide](https://github.com/iodide-project/pyodide).

## Enabling Python support

Python support is **not enabled by default**. To enable the Python runtime you must set up the relevant
environment variables:

```bash
# On the "upload" container/ endpoint (see docker-compose.yml locally)
PYTHON_CODEGEN=on
```

The first time the system runs it will generate machine code for CPython and all the 
Python C-extensions. This can take around a minute. 

## Running a Python function

An example Python function is found at `func/python/hello.py`. This can be uploaded 
and invoked from the Faasm CLI with:

```bash
inv upload python hello --py
inv invoke python hello --py
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

A couple of the Python test functions also demonstrate chaining:

- [Simple example](../func/python/chain.py) 
- [Chaining with shared state](../func/python/dict_state.py) 

# Building CPython

Note that this is only relevant for building the Python support from scratch.

## Python 3.7

We need a native version of exactly the right Python 3.7, _not_ necessarily the latest.
You need to install it with the `python3_7.yml` playbook. It will install a copy of python
at `/usr/local/faasm/python3.7`

```
cd ansible
ansible-playbook python3_7.yml
```

## Building CPython and packages

You must make sure that you've set up the latest toolchain, build libc and malloc etc. (see local dev docs).

Once this is done, you can run the following:

```
cd third-party/pyodide
source workon.sh
cd cpython
make clean && make
cd ../packages
make clean && make
```

## Setting up the runtime environment

Once this is all built we can put the relevant files in place _in a new terminal session_ at the root of the project:

```
inv python.set-up-runtime
```

## Adding packages

Adding packages to Pyodide is described [in their docs](https://github.com/iodide-project/pyodide/blob/master/docs/new_packages.md). For just a pure Python package you can do the following:

```
cd third-party/pyodide
source workon.sh
./bin/pyodide mkpkg <pypi pkg name>
```

If this doesn't work you can do the following:

- Create a new folder in `pyodide/packages`
- Copy the `meta.yml` from another pure Python package (e.g. `perf`)
- Add the right version, SHA and a link to the `.tar.gz` from PyPI (perf example [here](https://pypi.org/project/perf/))
- From the `packages` directory run `../bin/pyodide buildpkg --package_abi=0 <your_pkg>/meta.yaml`

This will automatically create a basic `meta.yml`.

You can then build it with:

```
./bin/pyodide buildpkg --package_abi=0 packages/<pypi pkg name>/meta.yaml
```

### Adding the new package to the runtime root

Once added, we need to include this in the Faasm runtime root:

- Open `tasks/python.py`
- Add a new entry in the dictionary of Python packages in there
- Make sure you use the right file path to the built package in pyodide

You can then set it up with:

```bash
inv python.set-up-package <pkg name>
```

Note that this modifies the runtime root bundle included in a release, so changes here will need to 
be reflected in a new release.

