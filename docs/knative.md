# Knative

## Uploading a function



## Deployment

Once you have a Kubernetes cluster with Knative set up (and can run `kubectl` to access it), you can deploy the Faasm functions doing the following:

```
./bin/knative_deploy.sh
```

To get the route to the function you then run:

```
./bin/knative_route.sh
```

## Troubleshooting

You can look at the logs for the faasm containers with something like:

```
# Find the faasm-test-xxx pod
kubectl get pods --namespace=faasm

# Get the logs for the user-container part and queue proxy
kubectl logs faasm-test-xxx user-container --namespace faasm
kubectl logs faasm-test-xxx queue-proxy --namespace faasm
```
