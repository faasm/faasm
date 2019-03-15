# Python 

## Python 3.7

If you don't already have python3.7 available, you need to install it with the `python3_7.yml` playbook,

```
cd ansible
ansible-playbook python3_7.yml --ask-become-pass
```

## Building CPython

We build Python using [pyodide](https://github.com/iodide-project/pyodide). This can be done by running:

```
inv build-python-pyodide
```

**NOTE** this will have to build their patched `emsdk` to work, so will take a long time on the first run.

## Running

To run Python you need to set up the Faasm runtime root filesystem. To do this you can run:

```
sudo mkdir /usr/local/faasm/runtime_root
sudo chown -R <user>:<user> !$
inv set-up-python-root
```