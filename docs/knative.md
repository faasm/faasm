# Knative

Once you have a Kubernetes cluster with Knative set up (and can run `kubectl` to access it), you can deploy the Faasm functions doing the following:

```
./bin/deploy_knative.sh
```

To get the route to the function you then run:

```
./bin/knative_route.sh
```

