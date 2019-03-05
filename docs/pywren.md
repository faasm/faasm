# Pywren

## Private key

To use EC2 you also need to create a private SSH key in AWS called `faasm_key` and store it at `~/faasm_key.pem`
(with `0600` perms).

## Components

The `pywren.yml` playbook will set up the relevant Pywren config file to match the components it creates.
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
