# Typescript 

Typescript WebAssembly support is called [assemblyscript](https://github.com/AssemblyScript/assemblyscript).

## Set-up

To set up the assemblyscript tools you can use the Ansible playbook in this repo:

```
cd ansible 
ansible-playbook assemblyscript.yml --ask-become-pass
```

This will install both nodejs and assemblyscript.

## Building functions

To build Typescript functions to wasm you can run:

```
cd assemblyscript
npm run asbuild
```

