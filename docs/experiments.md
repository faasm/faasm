# Experiments

Experiments are run on a Kubernetes cluster. Commands must be run from a machine with access to the cluster
that can run `kubectl` and `kn`.

Everything must be cleared away between runs to make sure stuff doesn't bleed across.

## Set-up

To run throughput/ latency experiments you'll need to install `wrk` with:

```bash
cd ansible
ansible-playbook wrk.yml
```

## Billing Estimates

To get resource measurements from the hosts running experiments we first need an inventory file at
`ansible/inventory/billing.yml`, something like:

```yaml
[all]
myhost1
myhost2
...
```

Then we can run the set-up with:

```bash
cd ansible
ansible-playbook -i inventory/billing.yml billing_setup.yml
```

## Data

Data should be generated and uploaded ahead of time.

### SGD

For details of the SGD experiment data see `sgd.md` notes.

### Matrices

The matrix experiment data needs to be generated in bulk locally, uploaded to S3 then downloaded on the client machine (or directly copied with `scp`). You must have the native tooling and pyfaasm installed to generate it up front (but
this doesn't need to be done if it's already in S3):

```bash
# Generate it
inv install-native-tools
inv generate-all-matrix-data

# Direct SCP from local machine
export HOST=<your_host>
export HOST_USER=<user_on_your_host>
scp -r ~/faasm/data/matrix $HOST_USER@$HOST:/home/$HOST_USER/faasm/data

# Upload (note - >4GB)
inv matrix-upload-s3

# Download
inv matrix-download-s3
```

### Tensorflow

Tensorflow data consists of the model and images. To download it (one-off) you can run:

```bash
./bin/download_tf_data.sh
```

Then to upload to your Faasm instance:

```bash
inv tf-upload-data tf-state-upload
```

## SGD Experiment

```bash
# -- Prepare --
# Upload data (one off)
inv reuters-state-upload

# -- Build/ upload --
inv build-knative-native sgd reuters_svm
inv upload sgd reuters_svm --prebuilt

# -- Deploy --

# Vary number of workers on each run
export N_WORKERS=10

# Native containers
inv deploy-knative-native sgd reuters_svm $N_WORKERS

# Wasm
inv deploy-knative $N_WORKERS

# -- Wait --

watch kn -n faasm service list
watch kubectl -n faasm get pods

# -- Run experiment --

# Native SGD
inv sgd-experiment --native $N_WORKERS 60000

# Wasm SGD
inv sgd-experiment $N_WORKERS 60000

# -- Clean up --

# Native SGD
inv delete-knative-native sgd reuters_svm

# Wasm
inv delete-knative-worker --hard
```

## Matrices Experiment

```bash
# -- Build/ Upload --
inv build-knative-native-python
inv upload python mat_mul --py

# Number of workers kept the same throughout
export N_WORKERS=<number of workers>

# -- Deploy --

# Native
inv deploy-knative-native-python $N_WORKERS

# Wasm
inv deploy-knative $N_WORKERS

# -- Run experiment --

# Native
inv matrix-experiment-multi $N_WORKERS --native

# Wasm
inv matrix-experiment-multi $N_WORKERS
```

## Tensorflow Experiment

When deploying the Tensorflow experiments, we need to restrict resources to make sure we can get to a point of saturating the system.

```bash
# -- Build/ upload --
inv build-knative-native tf image
inv upload tf image --prebuilt

# -- Upload data (one-off)
inv tf-upload-data tf-state-upload

# -- Deploy --
# NOTE: need to switch off Python preload and set "NO_PRE_CODEGEN=on"

# Native
inv deploy-knative-native tf image 16

# Wasm
inv deploy-knative 4

# -- Run experiment --

# Native
inv tf-tpt-experiment-multi --native

# Wasm
inv tf-tpt-experiment-multi
```

## Results

Once you've done several runs, you need to pull the results to your local machine and process:

```bash
# SGD
inv sgd-pull-results <user> <host>

# Matrices
inv matrix-pull-results <user> <host>
```
