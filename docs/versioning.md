# Versioning

Version needs to be updated in:

- `VERSION` at the project root
- `FAASM_VERSION` variable in `.env` file at project root
- Any kubernetes config files in `k8s` that specify image names

Docker images will also expect a numbered version of the runtime root, sysroot and toolchain. If these have changed since the last release you need to create new versions of them with the relevant `backup_xxx` commands. 

If they haven't changed, you can copy the old versions by running the following _after_ you've updated the `VERSION` file:

```bash
inv copy-release-bundles <old version>
```
