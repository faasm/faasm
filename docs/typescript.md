# Typescript 

Typescript WebAssembly support is added through [assemblyscript](https://github.com/AssemblyScript/assemblyscript).

## Set-up

To set up the assemblyscript tools you can use the Ansible playbook in this repo:

```
cd ansible 
ansible-playbook assemblyscript.yml --ask-become-pass
```

## Building functions

The Typescript functions live at `typescript/assembly`. 

To build Typescript the `echo` function you can run:

```
source workon.sh
inv compile ts echo --ts
```

You can then upload them to Faasm (provided an upload server is running):

```
inv upload ts echo --ts
```

