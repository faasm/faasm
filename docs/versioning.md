# Versioning

Version needs to be updated in:

- `VERSION` at the project root
- `FAASM_VERSION` variable in `.env` file at project root
- Any kubernetes config files in `k8s` that specify image names

Then:

- Update the versioned runtime root, sysroot and toolchain bundles (see below)
- Rebuild all Docker images (`inv docker-build-release`) 

## Sysroot, toolchain and runtime root

If these haven't changed since the last version, you can run the following 
_after_ you've updated the `VERSION` file:

```bash
inv copy-release-bundles <old version>
```

If they _have_ changed, you'll need to make sure you've got up-to-date versions
locally, then run the relevant `backup_xxx` commands. It's worth reading through the `docs/toolchain.md`
notes if you need to change the toolchain/ core libraries themselves.

