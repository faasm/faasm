# Experiments

Experiments are run on a Kubernetes cluster. Commands must be run from a machine with access to the cluster
that can run `kubectl` and `kn`.

Everything must be cleared away between runs to make sure stuff doesn't bleed across.

## Data

Data should be generated and uploaded ahead of time.

For details of the SGD experiment data see `sgd.md` notes.

The matrix experiment data needs to be generated in bulk locally, uploaded to S3 then downloaded on the client machine.

You must have the native tooling and pyfaasm installed to generate it up front (but this doesn't need to be done
if it's already in S3):

```
# Generate it
inv install-native-tools
inv generate-all-matrix-data

# Upload (note - >4GB)
inv matrix-upload-s3
```

Then on the client machine:

```
# With S3 access
inv matrix-download-s3

# Direct SCP from local machine
scp -r ~/faasm/data/matrix <USER>@<HOST>:/home/<USER>/faasm/data/matrix
```

## Executing

We need to run the native version and the wasm version. This looks like:

```
# ---- Data upload ----

# SGD
inv reuters-state-upload

# ---- Deploy containers ----

# Native SGD
inv deploy-knative-native sgd reuters_svm 10

# Native matrices
inv deploy-knative-native-python 10

# Wasm
inv deploy-knative 10

# ---- Wait ----
watch kn -n faasm service list
watch kubectl -n faasm get pods

# ---- Run ----

# Native SGD
inv sgd-experiment --native 10 60000

# Wasm SGD
inv sgd-experiment 10 60000

# Native matrices
inv matrix-state-upload 1000 3
inv matrix-experiment --native 1000 3

# Wasm matrices
inv matrix-state-upload 1000 3
inv matrix-experiment 1000 3

# ---- Clean up ----

# Native SGD
inv delete-knative-native sgd reuters_svm

# Native matrices
inv delete-knative-native-python

# Wasm
inv delete-knative-worker
```

## Results

Once you've done several runs, you need to pull the results to your local machine and process:

```
# SGD
inv sgd-pull-results <user> <host>
inv sgd-parse-results

# Matrices
inv matrix-pull-results <user> <host>
inv matrix-parse-results
```
