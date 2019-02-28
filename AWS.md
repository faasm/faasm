# Faasm on AWS

## IAM user

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

## Component set-up

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

## Lambda functions

To set up the requried Lambda functions, you can run the following task which will build, upload and configure
the relevant AWS Lambda functions needed to run Faasm:

```
inv deploy-faasm-lambda
```

## Pywren

### Private key

To use EC2 you also need to create a private SSH key in AWS called `faasm_key` and store it at `~/faasm_key.pem`
(with `0600` perms).

### Components

The above playbook will set up the relevant Pywren config file to match the components it creates.
This is found at `~/.pywren_config`.

You clone the Pywren code from [this fork](https://github.com/Shillaker/pywren) and set up as an
editable module:

```
python3 -m venv venv
source venv/bin/activate
pip install -e ./
```

You won't need to run the full set-up, only the following commands:

```
pywren create_role
pywren deploy_lambda
```

### Stand-alone set-up

To experiment with Pywren standalone mode, you can set up hosts with:

```
pywren standalone launch_instances 1
```
