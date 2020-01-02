# Versioning

Version needs to be updated in:

- `VERSION` at the project root
- `FAASM_VERSION` variable in `.env` file at project root
- Any kubernetes config files in `k8s` that specify image names

Docker images will also expect a numbered version of the runtime root, sysroot and toolchain. If these have changed since the last release you need to create new versions of them with the relevant commands. If not you can just rename them in the [`faasm-misc` bucket](https://s3.console.aws.amazon.com/s3/buckets/faasm-misc/?region=eu-west-1&tab=overview). This process needs improving.
