# LLVM WebAssembly Support

LLVM 8 makes WebAssembly support official. 

To install LLVM you can run the following Ansible task:

```
cd ansible
ansible-playbook clang8.yml --ask-become-pass
```

## Musl

To install musl, you need to run the following:

```
./bin/build_musl.sh
```