# Faasm with WAVM

## Set-up

Local set-up should be covered in the Ansible files in this project.

### Local dev machine

```
cd ansible
ansible-playbook libs.yml dev.yml
ansible-playbook k8_local
```

You can then try the example on the [minikube tutorial](https://kubernetes.io/docs/setup/minikube/).

### Note re. Binaryen

Running with Binaryen is currently **broken** but requires the `llvm.yml`, `binaryen.yml` and potentially `musl.yml`
playbooks too.
