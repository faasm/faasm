# Faasm Releases

## 0. Create a branch

Create a new branch for this release, but _not_ a PR.

See what's going into the release in the [Compare
view](https://github.com/faasm/faasm/compare).

## 1. Prepare release artifacts

The sysroot, toolchain and runtime root are all tied to a given release. You
must create the release versions locally before proceeding. 

To do this, you can download the versions from the current release, then apply
any changes for this release (if there are any), i.e.:

```bash
inv toolchain.download-toolchain
inv toolchain.download-sysroot
inv toolchain.download-runtime
``` 

To check things are working properly you can run the tests locally with the new
versions.

It's worth reading through the
[toolchain](https://github.com/faasm/faasm-toolchain/README.md)
notes if you need to change the `faasm-toolchain` core libraries themselves.

## 2. Update version

Version can be updated with:

```
# Minor 
inv release.bump

# Custom
inv release.bump --ver=1.2.3
```

Check the diff to make sure this hasn't edited anything we don't expect.

Push all the changes to your branch.

## 3. Create the Github release

Run the following to create the new release in Github (this will bundle up the
sysroot, toolchain and runtime root too).

NOTE: this will create a tag at the head of your current branch.

```bash
inv release.tag
inv release.create
inv release.upload
```

Now check the draft release. If it's ok:

```bash
inv release.publish
```

## 4. Rebuild Docker containers

You can rebuild all the containers for the given release with:

```bash
inv docker.release
```

Note that the `base-test` container will be built from the tag you just created
so this is what the CI build will use too.

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
