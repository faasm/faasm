# HTTP API

Most interactions with Faasm can be performed through the [CLI](cli.md),
which itself uses the [`faasmctl`](https://github.com/faasm/faasmctl) API.

You should never need to interact with the underlying HTTP API directly.
If you still need to do so for debugging or hacking purposes, we recommend
checking the implementation in [`faasmctl`](
https://github.com/faasm/faasmctl/tree/main/faasmctl/util).

Note that the API currently has no authentication.

## Upload WASM

The upload server is used to upload WASM functions to a running Faasm cluster.
Each function is identified by a `user/function` pair. Once uploaded, functions
can be invoked by passing the `user` and the `function`.

```bash
faasmctl upload.wasm <user> <function> <path_to_wasm_file>
```

## Upload shared files

Files can be made accessible to functions through the virtual filesystem.
A file uploaded at `<faasm_path>` is accessible to uploaded functions at
`faasm://<faasm_path>`.

To upload a file to the virtual filesystem you may run:

```bash
faasmctl upload.file <host_path> <faasm_path>
```

## Invoke WASM functions

In order to invoke a WASM function (previously uploaded with `user/func`) you
may run:

```bash
faasmctl invoke <user> <func>
```
