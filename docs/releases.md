# Faasm Releases

## 0. Create a branch

Create a new branch for this release, but _not_ a PR.

See what's going into the release in the [Compare
view](https://github.com/faasm/faasm/compare).

## 1. Update version

Version can be updated with:

```
# Minor 
inv release.bump

# Custom
inv release.bump --ver=1.2.3
```

Check the diff to make sure this hasn't edited anything we don't expect.

Push all the changes to your branch.

## 2. Create the Github release

Run the following to create the new release in Github.

NOTE: this will create a tag at the head of your current branch.

```bash
inv release.tag
inv release.create
inv release.publish
```

Note that you can force overwriting of a tag with:

```bash
inv release.tag --force
```

## 3. Rebuild Docker containers

You can rebuild all the containers for the given release with:

```bash
inv docker.release
```

Note that the `base-test` container will be built from the tag you just created
so this is what the CI build will use too.

## 4. Create a PR

Create a PR from your branch, this will then run through the tests for the new
release. If it's green, you can merge it into master.

# Github config

If this is your first time releasing, you'll need to 
[create a public access token](https://github.com/settings/tokens) 
with the relevant permissions (just `public_repo`) and add it to 
`~/faasm/faasm.ini`:

```ini
[Github]
access_token: <token>
``` 
