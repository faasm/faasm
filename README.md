# Faasm

Use Ansible playbooks to set up locally.

Makefile has useful commands for running and set-up

# Setup

## Minikube

It can be a bit of a hassle to get Minikube running. Steps are:

- Run the `minikube.yml` playbook
- Log out/ reboot
- Start minikube with `make minikube-start`
- Wait for ages as it does its thing

If there are errors, run `make minikube-reset`.