# Experiments

Experiments are run on a Kubernetes cluster. Commands must be run from a machine with access to the cluster
that can run `kubectl` and `kn`.

Everything must be cleared away between runs to make sure stuff doesn't bleed across.

## Data

The SGD experiment data must be set up once ahead of time. See the `sgd.md` notes.

## Executing

We need to run the native version and the wasm version. This looks like:

```
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
inv matrix-experiment --native 10000 3

# Wasm matrices
inv matrix-experiment 10000 3

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
