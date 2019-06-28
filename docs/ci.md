# Continuous Integration Set-up

CI builds can be found on circle at https://circleci.com/gh/lsds/Faasm

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



