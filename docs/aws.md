# Faasm on AWS

## Invocation

Once the Faasm Lambda functions are deployed (as described below), you can do the following:

```
# Upload a .wasm file (after having built locally). Will run codegen after
inv deploy-wasm-lambda-func <user> <func>

# Invoke the function
inv invoke-faasm-lambda <user> <func>

# Upload the function as native code
inv deploy-native-lambda-func <user> <func>

# Invoke the native function
inv invoke-lambda <lambda_name>
```

## Set-up

### AWS CLI

Assuming you've already installed `awscli` (can be done with `pip3`), you need to run:

```
aws configure
```

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
# Set up the infra
cd ansible
ansible-playbook aws_infra.yml

# Set up local config file
cd ..
inv generate-aws-config
```

You can then use the corresponding task to tear down the components (Elasticache can be expensive):

```
ansible-playbook aws_teardown.yml
```

## Functions

### AWS Lambda CPP runtime

Before deploying you must have the [Lambda CPP runtime](https://github.com/awslabs/aws-lambda-cpp) installed.

### Faasm Lambda functions

To set up the requried Lambda functions for running WebAssembly, you can run the following task which will
build, upload and configure the relevant AWS Lambda functions needed to run WebAssembly with Faasm:

```
inv deploy-faasm-lambda
```

To delete them before or after, you can run:

```
inv delete-faasm-lambda
```

### SGD

There are a couple of shortcuts for setting up the SGD experiments:

```
# Deploy wasm functions
inv deploy-sgd-wasm-lambda

# Deploy native functions
inv deploy-sgd-native-lambda
```
