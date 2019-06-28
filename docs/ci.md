# Continuous Integration

CI builds can be found on circle at https://circleci.com/gh/lsds/Faasm

## Checking Locally

To check things locally you can run the following:

```
./bin/ci_check.sh
```

This sets up the test container locally and mounts your local checkout. Inside the container you need to run:

```
/usr/local/code/faasm/bin/ci_setup.sh

cd /build/faasm
./bin/tests

# After making changes
cmake --build . --target tests && /faasm/build/bin/tests
```

It's worth double checking the `.circleci/config.yml` file to make sure the local set-up still matches.

# Set-up

## Build Requirements

A couple of things are needed to get the build to run.

### `faasm/testing` image

The `faasm/testing` Docker image must be built and pushed to Dockerhub for the build to run. This image only needs to
be updated if there are changes to the environment outside of the core code (e.g. added a new dependency or
upgraded WAVM). If that is the case you need to make sure `faasm/base` image and `faasm/worker` include the latest
stable changes.

These can all be managed with the `build-xxx` and `push-xxx` commands, e.g. `inv build-testing push-testing`.

### Python runtime root

The Python runtime must be packaged and pushed to S3 as detailed in the Python docs.

### `.wasm` files

Some tests rely on actually executing wasm files. The `.wasm` files are checked into the repo where
necessary, but codegen is run on them inside the build container before executing the tests.



