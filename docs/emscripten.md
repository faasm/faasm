# Emscripten

To set up Emscripten you can run the following:

```
cd ansible
ansible-playbook emscripten.yml
```

## Activiating the relevant Emscripten

If you have multiple Emscriptens installed on your system, you need to source `emsdk_env.sh`
from your chosen distribution.

## Compiling functions

To compile functions in the `func` dir we need to build the required libraries, then the functions themselves:

```
inv compile-libfaasm-emscripten
inv funcs-emscripten
```