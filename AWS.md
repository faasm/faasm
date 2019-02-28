# Faasm on AWS

## Set-up

### IAM user

Before running anything you'll need to create a new IAM user with the following roles:

- AmazonEC2FullAccess
- AmazonS3FullAccess
- AmazonSQSFullAccess
- AWSLambdaFullAccess
- IAMFullAccess
- AmazonElastiCacheFullAccess

Their credentials then need to be added to your `~/.aws/credentials` file under the `default` profile, i.e.

```
[default]
aws_access_key_id = <public_key>
aws_secret_access_key = <private_key>
```

### Component set-up

There's an Ansible task that will set up all the relevant bits. You need to modify `region` and `aws_account_id`
in `ansible/roles/aws/defaults/main.yml` to match your account, then run:

```
cd ansible
ansible-playbook aws_infra.yml
```

You can then use the corresponding task to tear down the components (Elasticache can be expensive):

```
ansible-playbook aws_teardown.yml
```

## Functions

### Faasm Lambda functions

To set up the requried Lambda functions for running WebAssembly, you can run the following task which will
build, upload and configure the relevant AWS Lambda functions needed to run WebAssembly with Faasm:

```
inv deploy-faasm-lambda
```

### Uploading native functions

Functions in the `func` directory can be compiled to machine code and deployed as native lambda functions
with the `deploy-native-lambda-func` task, e.g. `inv deploy-native-lambda-func demo echo`.

### Uploading wasm functions

To upload functions as wasm, you first need to run the functions wasm build locally (`inv funcs` from inside the
toolchain container).

Once this is done, you can deploy a given function to lambda with `inv deploy-wasm-lambda-func`, e.g.
`inv deploy-wasm-lambda-func demo echo`. This will upload the function to S3, then use another lambda function to
run the code generation.
