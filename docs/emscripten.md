# Emscripten

To set up Emscripten you can run the following:

```
cd ansible
ansible-playbook emscripten.yml
```

## Compiling functions

To compile functions in the `func` dir we need to build the required libraries, then the functions themselves:

```
inv compile-libfaasm-emscripten
inv funcs-emscripten
```