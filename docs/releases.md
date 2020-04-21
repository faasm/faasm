# Faasm Releases

## 0. Create a branch

Create a new branch for this release, but _not_ a PR.

See what's going into the release in the [Compare view](https://github.com/lsds/Faasm/compare).

## 1. Prepare release artifacts

The sysroot, toolchain and runtime root are all tied to a given release. You must create the 
release versions locally before proceeding. 

To do this, you can download the versions from the current release, then apply any changes 
for this release (if there are any), i.e.:

```bash
inv toolchain.download-toolchain
inv toolchain.download-sysroot
inv toolchain.download-runtime
``` 

To check things are working properly you can run the tests locally with the new versions.

It's worth reading through the `docs/toolchain.md` notes if you need to change the 
toolchain/ core libraries themselves.

## 2. Update version

Version needs to be updated in:

- `VERSION` at the project root
- `FAASM_VERSION` variable in `.env` file at project root
- Any kubernetes config files in `deploy/k8s` that specify image names

This can usually be done with a find-and-replace on the current version number.

## 3. Create the Github release

Run the following to create the new release in Github (this will bundle up the sysroot,
toolchain and runtime root too).

```bash
inv github.create-release
inv github.upload-artifacts
```

Now check the draft release. If it's ok:

```bash
inv github.publish-release
```

## 4. Rebuild Docker containers

You can rebuild all the containers for the given release with:

```bash
inv docker.release
```

## 5. Create a PR

Create a PR from your branch, this will then run through the tests for the new
release. If it's green, you can merge it into master.

## Github config

If this is your first time releasing, you'll need to 
[create a public access token](https://github.com/settings/tokens) 
with the relevant permissions (just `public_repo`) and add it to 
`~/faasm/faasm.ini`:

```ini
[Github]
access_token: <token>
``` 