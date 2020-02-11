# Faasm Releases

## Versioning

Version needs to be updated in:

- `VERSION` at the project root
- `FAASM_VERSION` variable in `.env` file at project root
- Any kubernetes config files in `k8s` that specify image names

Then:

- Update the versioned runtime root, sysroot and toolchain bundles (see below)
- Rebuild all Docker images (`inv docker-build-release`) 

## Release artifacts

Be careful that your local sysroot, toolchain and runtime root reflect what should go into the 
release. To do this you can download the versions from the _last_ release, then apply any 
changes for this release (if there are any), i.e.:

```bash
inv download-toolchain
inv download-sysroot
inv download-runtime-root
``` 

Your local versions will get bundled up and included in the new release.

It's worth reading through the `docs/toolchain.md` notes if you need to change the 
toolchain/ core libraries themselves.

## Commands

Note: you must make sure you've _already_ updated the versions where necessary.

```bash
inv gh-create-release
inv gh-upload-artifacts
```

Now check the draft release. If it's ok:

```bash
inv gh-publish-release
```

## Github config

You'll need to create a public access token with the relevant permissions and 
add it to `~/faasm/faasm.ini`:

```ini
[Github]
access_token: <token>
``` 