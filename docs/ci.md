# Continuous Integration Set-up

CI builds can be found on circle at https://circleci.com/gh/lsds/Faasm

## Build Requirements

A couple of things are needed to get the build to run.

### Faasm test container image

The `faasm/testing` Docker image must be built and pushed to Dockerhub for the build to run. This image only needs to
be updated if there are changes to the environment outside of the core code (e.g. added a new dependency or
upgraded WAVM). If that is the case you need to make sure `faasm/base` image and `faasm/worker` include the latest
stable changes.

These can all be managed with the `build-xxx` and `push-xxx` commands, e.g. `inv build-testing push-testing`.

### Python runtime root

The Python runtime must be packaged and pushed to S3 as detailed in the Python docs.

### `.wasm` files

Some tests rely on actually executing wasm files. These are accessible from the S3
[`faasm-runtime`](https://s3.console.aws.amazon.com/s3/buckets/faasm-runtime/wasm) bucket.

To update these files you can run:

```
# Compile all funcs
inv compile --clean

# Upload all wasm to S3
inv upload-all-s3
```



