# Faasm Releases

## 0. Create a branch

Create a new branch for this release, and a draft PR (or no PR at all).

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

## 2. Tag the release

Each release is built from a tag. Create this with:

```bash
inv release.tag
```

This will trigger the [container
build](https://github.com/faasm/faasm/actions?query=workflow%3ARelease).

If there's an issue with the tag (e.g. the container build fails), you can
update it with:

```bash
inv release.tag --force
```

If you're having real issues, see the section below on building containers
locally.

## 3. Create a PR

Create a PR from your branch, this will then run through the tests for the new
release. If it's green, you can merge it into master.

## 4. Re-tag after the PR is merged

It is encouraged to re-tag the code once the PR has been merged to keep a clean
commit log. To do so, once the PR has been merged:

```bash
git checkout master
git pull origin master
cat VERSION # ensure VERSION has the correct tag
inv git.tag --foce
```

## 5. Create the Github release

Run the following to create the new release from your tag:

```bash
inv release.create
inv release.publish
```

# Building containers locally

Usually we rely on Github to build containers, but if you need to build them
locally you can use:

```bash
inv docker.build -c <container>
```

To build more than one:

```bash
inv docker.build -c <containerA> -c <containerB>
```

In particular, if you have re-tagged the code and the builds in github are not
working, this line will get all the container images up to speed (will take a
while though):

```bash
inv docker.build -c base -c worker -c cli -c upload --push --nocache
```

# Github config

If this is your first time releasing, you'll need to 
[create a public access token](https://github.com/settings/tokens) 
with the relevant permissions (just `public_repo`) and add it to 
`~/faasm/faasm.ini`:

```ini
[Github]
access_token: <token>
``` 
