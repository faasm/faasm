# Faasm with WAVM

## Set-up

Local set-up should be covered in the Ansible files in this project.

To run with Emscripten you need to do:

```
cd ansible
ansible-playbook clang.yml emscripten.yml wavm.yml --ask-become-pass
```

Running with Binaryen is currently **broken** but requires the `llvm.yml`, `binaryen.yml` and potentially `musl.yml`
playbooks too.

## Compiling and running

The `compile.sh` script will run `emcc` to compile the function.
