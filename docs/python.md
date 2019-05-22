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

You must make sure that you've set up the latest toolchain, build libc and malloc etc.

Once this is done, you can run the following:

```
cd pyodide
source workon.sh
cd cpython
make
cd ../packages
make
```

## Setting up the runtime environment

Once this is all built we can generate the machine code and put everything in place as follows:

```
inv set-up-python-runtime
```

## Compiling the Python function

When building you **must** use the Faasm shell environment:

```
source workon.sh
```

Once this is done, you can build and upload the Python function as follows (assuming you have an upload server running)

```
inv compile --func=py_func
inv upload python py_func
```

## Adding packages

Adding packages to Pyodide is described [in their docs](https://github.com/iodide-project/pyodide/blob/master/docs/new_packages.md). For just a pure Python package you can do the following:

- Create a new folder in `pyodide/packages`
- Copy the `meta.yml` from another pure Python package (e.g. `perf`)
- Add the right version, SHA and a link to the `.tar.gz` from PyPI (perf example [here](https://pypi.org/project/perf/))
- From the `packages` directory run `../bin/pyodide buildpkg --package_abi=0 <your_pkg>/meta.yaml`
