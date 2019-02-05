# Faasm on AWS

## IAM user

You need to create a new IAM user called `faasm_admin` with the following roles:

- AmazonEC2FullAccess
- AmazonS3FullAccess
- AmazonSQSFullAccess
- AWSLambdaFullAccess
- IAMFullAccess

Their credentials need to be added to your `~/.aws/credentials` file under `faasm`, i.e.

```
[faasm]
aws_access_key_id = <public>
aws_secret_access_key = <private>
```

## Private key

You need to create a private SSH key in AWS called `faasm_key` and store it at `~/faasm_key.pem` (with 0600 perms).

## Components

You can use the Ansible task in this repo to set up the required components (you'll need to change the account
ID in `aws.yml`):

```
cd ansible
ansible-playbook aws.yml
```

## Pywren

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
