# Python 

## Emscripten

You need to set up Emscripten with the ansible playbook, `emscripten.yml`.

## Building CPython

We build Python using [pyodide](https://github.com/iodide-project/pyodide). This can be done by running:

```
inv build-python-emscripten
```

## Compiling functions


To compile functions in the `func` dir we need to build the required libraries, then the functions themselves:

```
inv compile-libfaasm-emscripten
inv funcs-emscripten
```

## Running

To run Python you need to set up the Faasm runtime root properly. To do this you can run:

```
sudo mkdir /usr/local/faasm/runtime_root
sudo chown -R <user>:<user> !$
inv set-up-python-root
```