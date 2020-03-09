# Experiments

Experiments are run on a Kubernetes cluster. Commands must be run from a machine with access to the cluster
that can run `kubectl` and `kn`.

Everything must be cleared away between runs to make sure stuff doesn't bleed across.

## Client Machine Set-up

To run throughput/ latency experiments you'll need to set up the client machine with (on the machine itself):

```bash
cd ansible
ansible-playbook load_client.yml
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
inv libs.native
inv matrix-data.generate-all

# Direct SCP from local machine
export HOST=<your_host>
export HOST_USER=<user_on_your_host>
scp -r ~/faasm/data/matrix $HOST_USER@$HOST:/home/$HOST_USER/faasm/data

# Upload (note - >4GB)
inv data.matrix-upload-s3

# Download
inv data.matrix-download-s3
```

### Tensorflow

Tensorflow data consists of the model and images. These need to be 
uploaded to your Faasm instance:

```bash
inv data.tf-upload data.tf-state
```

## SGD Experiment

```bash
# -- Prepare --
# Upload data (one off)
inv data.reuters-state

# -- Build/ upload --
inv knative.build-native sgd reuters_svm
inv upload sgd reuters_svm

# -- Deploy --

# Vary number of workers on each run
export N_WORKERS=10

# Native containers
inv knative.deploy-native sgd reuters_svm $N_WORKERS

# Wasm
inv knative.deploy $N_WORKERS

# -- Wait --

watch kn -n faasm service list
watch kubectl -n faasm get pods

# -- Run experiment --

# Native SGD
inv experiments.sgd --native $N_WORKERS 60000

# Wasm SGD
inv experiments.sgd $N_WORKERS 60000

# -- Clean up --

# Native SGD
inv knative.delete-native sgd reuters_svm

# Wasm
inv knative.delete-worker --hard
```

## Matrices Experiment

```bash
# -- Build/ Upload --
inv upload python mat_mul --py

# Number of workers kept the same throughout
export N_WORKERS=<number of workers>

# -- Deploy --

# Native
inv knative.deploy-native-python $N_WORKERS

# Wasm
inv knative.deploy $N_WORKERS

# -- Run experiment --

# Native
inv experiments.matrix-multi $N_WORKERS --native

# Wasm
inv experiments.matrix-multi $N_WORKERS
```

## Tensorflow Experiment

You need to set the following environment variables for these experiments (through the knative config):

- `COLD_START_DELAY_MS=800`
- `TF_CODEGEN=on`
- `SGD_CODEGEN=off`
- `PYTHON_CODEGEN=off`
- `PYTHON_PRELOAD=off`

Preamble:

```bash
# -- Build/ upload --
inv knative.build-native tf image
inv upload tf image

# -- Upload data (one-off)
inv data.tf-upload data.tf-state
```

Latency:

```bash
# -- Deploy both (note small number of workers) --
inv knative.deploy-native tf image 1
inv knative.deploy 1

# -- Run experiment --
inv experiments.tf-lat
```

Throughput:

```bash
# -- Deploy --
# Native
inv knative.deploy-native tf image 30

# Wasm
inv knative.deploy 18

# -- Run experiment --

# Native 
inv experiments.tf-tpt --native

# Wasm latency
inv experiments.tf-tpt
```

## Results

Once you've done several runs, you need to pull the results to your local machine and process:

```bash
# SGD
inv experiments.sgd-pull-results <user> <host>

# Matrices
inv experiments.matrix-pull-results <user> <host>

# Inference latency
inv experiments.tf-lat-pull-results <user> <host>

# Inference throughput
inv experiments.tf-tpt-pull-results <user> <host>
```
