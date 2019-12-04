# Python 

## Using Python Support

Note that this doc is only relevant for building the Python support from scratch. This should only be necessary if
you plan on editing anything related. Otherwise you can just use the `local_dev.md` doc which tells you how to set 
up the prebuilt version.

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
source workon.sh
inv set-up-python-runtime
```

We then need to generate machine code for this with:

```
inv run-python-codegen
```

## Compiling the Python function

We use a single Faasm function to execute all Python functions. This can be built as follows:

```
source workon.sh
inv compile --func=py_func
inv upload python py_func
```

## Adding packages

Adding packages to Pyodide is described [in their docs](https://github.com/iodide-project/pyodide/blob/master/docs/new_packages.md). For just a pure Python package you can do the following:

- Create a new folder in `pyodide/packages`
- Copy the `meta.yml` from another pure Python package (e.g. `perf`)
- Add the right version, SHA and a link to the `.tar.gz` from PyPI (perf example [here](https://pypi.org/project/perf/))
- From the `packages` directory run `../bin/pyodide buildpkg --package_abi=0 <your_pkg>/meta.yaml`

You can also try their helper script which is just:

```
cd third-party/pyodide
bin/pyodide mkpkg <pypi_pkg>
```

This will automatically create a basic `meta.yml`.

## Packaging the Python runtime

To package the Python runtime for use on AWS and in containers, we can run the following:

```
inv package-python-runtime
```

_WARNING_ this nukes all the object files in your current `runtime_root` so you'll need to regenerate them and 
run tests to make sure it's worked.

This bundles up the required runtime files and uploads them to S3.
