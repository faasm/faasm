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

You must make sure that you've set up the latest emscripten toolchain locally, build libc and malloc etc.

Once this is done, you can run the following:

```
cd pyodide
source workon.sh
cd cpython
make
cd ../packages
make
```

## Adding packages

Adding packages to Pyodide is described [in their docs](https://github.com/iodide-project/pyodide/blob/master/docs/new_packages.md). For just a pure Python package you can do the following:

- Create a new folder in `pyodide/packages` 
- Copy the `meta.yml` from another pure Python package (e.g. `perf`)
- Add the right version, SHA and a link to the `.tar.gz` from PyPI (perf example [here](https://pypi.org/project/perf/))
- From the `packages` directory run `../bin/pyodide buildpkg --package_abi=0 <your_pkg>/meta.yaml`

## Dynamic linking

Once this is all built we can generate the machine code up-front for all the relevant shared objects.

```
./bin/python_codegen.sh
```

## Setting up the runtime root

To make everything available at runtime, you can execute the following:

```
./bin/set_up_python_root.sh
```

## Compiling Python functions

When building you **must** use the Faasm shell environment:

```
source workon.sh
```

Once this is done, you can run the `upload` server and do the following:

```
inv funcs --func=<your func>
inv upload-func <your user> <your func>
```
